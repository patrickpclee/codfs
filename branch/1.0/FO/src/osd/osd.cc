/**
 * osd.cc
 */

#include <thread>
#include <vector>
#include <openssl/md5.h>
#include <algorithm>
#include "osd.hh"
#include "../common/enumtostring.hh"
#include "../common/blocklocation.hh"
#include "../common/debug.hh"
#include "../common/define.hh"
#include "../common/metadata.hh"
#include "../common/convertor.hh"
#include "../config/config.hh"
#include "../protocol/status/osdstartupmsg.hh"
#include "../protocol/status/osdshutdownmsg.hh"
#include "../protocol/status/osdstatupdatereplymsg.hh"
#include "../protocol/status/newosdregistermsg.hh"
#include "../protocol/transfer/getblockinitrequest.hh"

// for random srand() time() rand() getloadavg()
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/statvfs.h>

// Global Variables
extern ConfigLayer* configLayer;
mutex segmentRequestCountMutex;
mutex recoveryMutex;

#include <atomic>

#ifdef PARALLEL_TRANSFER

#include "../../lib/threadpool/threadpool.hpp"
boost::threadpool::pool _blocktp;
boost::threadpool::pool _recoverytp;
#endif

using namespace std;

Osd::Osd(uint32_t selfId) {

    configLayer = new ConfigLayer("osdconfig.xml", "common.xml");
    _storageModule = new StorageModule();
    _osdCommunicator = new OsdCommunicator();
    _codingModule = new CodingModule();
    _osdId = selfId;

    srand(time(NULL)); //random test

#ifdef PARALLEL_TRANSFER
    uint32_t _numThreads = configLayer->getConfigInt("ThreadPool>NumThreads");
    _blocktp.size_controller().resize(_numThreads);
    _recoverytp.size_controller().resize(RECOVERY_THREADS);
#endif

    _reportCacheInterval = configLayer->getConfigLong(
            "Storage>ReportCacheInterval");

    _blocktpId = 0;
    _updateId = 0;
    _recoverytpId = 0;
}

Osd::~Osd() {
    //delete _blockLocationCache;
    delete _storageModule;
    delete _osdCommunicator;
}

void Osd::reportRemovedCache() {
    debug("%s\n", "Cache report thread started");

    while (true) {
        debug("%s\n", "Checking cache...");

        list<uint64_t> currentCacheList =
                _storageModule->getSegmentCacheQueue();

        // sort list
        currentCacheList.sort();

        // deleted segments (current - previous)

        debug_cyan("Old cache size = %zu, new cache size = %zu\n",
                _previousCacheList.size(), currentCacheList.size());

        list<uint64_t> deletedCacheList;
        set_difference(_previousCacheList.begin(), _previousCacheList.end(),
                currentCacheList.begin(), currentCacheList.end(),
                std::inserter(deletedCacheList, deletedCacheList.end()));

        // for debug
        string deletedCacheString;
        for (auto segmentId : deletedCacheList) {
            deletedCacheString += to_string(segmentId) + " ";
        }
        debug_yellow("Deleted Cache = %s\n", deletedCacheString.c_str());

        // report to MDS
        _osdCommunicator->reportDeletedCache(deletedCacheList, _osdId);

        _previousCacheList = currentCacheList;

        sleep(_reportCacheInterval);
    }
}

void Osd::cacheSegment(uint64_t segmentId, SegmentData segmentData) {

    if (!_storageModule->isSegmentCached(segmentId)) {
        _storageModule->putSegmentToDiskCache(segmentId, segmentData);
    }
}

void Osd::freeSegment(uint64_t segmentId, SegmentData segmentData) {
    // free segmentData
    debug("free segment %" PRIu64 "\n", segmentId);
    MemoryPool::getInstance().poolFree(segmentData.buf);
    debug("segment %" PRIu64 "free-d\n", segmentId);

}

/**
 * Send the segment to the target
 */

void Osd::getSegmentRequestProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, bool localRetrieve) {

    if (localRetrieve) {
        debug_yellow("Local retrieve for segment ID = %" PRIu64 "\n",
                segmentId);
    }

    segmentRequestCountMutex.lock();
    if (!_segmentRequestCount.count(segmentId)) {
        _segmentRequestCount.set(segmentId, 1);
        mutex* tempMutex = new mutex();
        _segmentDownloadMutex.set(segmentId, tempMutex);
        _segmentDataMap.set(segmentId, { });
        _isSegmentDownloaded.set(segmentId, false);
    } else {
        _segmentRequestCount.increment(segmentId);
    }
    struct SegmentData& segmentData = _segmentDataMap.get(segmentId);
    segmentRequestCountMutex.unlock();

    {
        lock_guard<mutex> lk(*(_segmentDownloadMutex.get(segmentId)));

        if (!_isSegmentDownloaded.get(segmentId)) {

            if (_storageModule->isSegmentCached(segmentId)) {
                // case 1: if segment exists in cache
                debug("Segment ID = %" PRIu64 " exists in cache", segmentId);

                // for local retrieve, no need to get it
                if (!localRetrieve) {
                    segmentData = _storageModule->getSegmentFromDiskCache(
                            segmentId);

#ifdef USE_SEGMENT_CACHE
                    // hack: trigger MDS to update hotness
                    _osdCommunicator->getSegmentInfoRequest(segmentId, _osdId, false);
#endif
                }

            } else {
                // case 2: if segment does not exist in cache

                // TODO: check if osd list exists in cache

                // 1. ask MDS to get segment information

                SegmentTransferOsdInfo segmentInfo =
                        _osdCommunicator->getSegmentInfoRequest(segmentId,
                                _osdId);

                const CodingScheme codingScheme = segmentInfo._codingScheme;
                const string codingSetting = segmentInfo._codingSetting;
                const uint32_t segmentSize = segmentInfo._size;

                // bool array to store osdStatus
                vector<bool> blockStatus =
                        _osdCommunicator->getOsdStatusRequest(
                                segmentInfo._osdList);

                // check which blocks are needed to request
                uint32_t totalNumOfBlocks = segmentInfo._osdList.size();
                block_list_t requiredBlockSymbols =
                        _codingModule->getRequiredBlockSymbols(codingScheme,
                                blockStatus, segmentSize, codingSetting);

                // error in finding required Blocks (not enough blocks to rebuild segment)
                if (requiredBlockSymbols.size() == 0) {
                    debug_error(
                            "Not enough blocks available to rebuild Segment ID %" PRIu64 "\n",
                            segmentId);
                    exit(-1);
                }

                // 2. initialize list and count

                const uint32_t blockCount = requiredBlockSymbols.size();

                _downloadBlockRemaining.set(segmentId, blockCount);
                _downloadBlockData.set(segmentId,
                        vector<struct BlockData>(totalNumOfBlocks));
                vector<struct BlockData>& blockDataList =
                        _downloadBlockData.get(segmentId);

                debug("PendingBlockCount = %" PRIu32 "\n", blockCount);

                // 3. request blocks
                // case 1: load from disk
                // case 2: request from OSD
                // case 3: already requested

                for (auto blockSymbols : requiredBlockSymbols) {

                    uint32_t i = blockSymbols.first;

                    uint32_t osdId = segmentInfo._osdList[i];

                    uint32_t parityCount = _codingModule->getParityNumber(codingScheme, codingSetting);
                    bool isParity = (i >= totalNumOfBlocks - parityCount);

                    if (osdId == _osdId) {

                        // read block from disk

                        BlockData blockData;

                        if (isParity) {
                            #ifdef APPEND_PARITY
                                #ifdef COMPACT_PARITY_ON_READ
                                _storageModule->mergeBlock(segmentId, i, true);
                                blockData = _storageModule->readBlock(
                                        segmentId, i, blockSymbols.second);
                                #else
                                blockData = _storageModule->getMergedBlock(segmentId, i, true);
                                #endif
                            #else
                                blockData = _storageModule->readBlock(
                                        segmentId, i, blockSymbols.second);
                            #endif
                        } else {
                            #ifdef APPEND_DATA
                                blockData = _storageModule->getMergedBlock(segmentId, i, false);
                            #else
                                blockData = _storageModule->readBlock(
                                        segmentId, i, blockSymbols.second);
                            #endif
                        }

                        // blockDataList reserved space for "all blocks"
                        // only fill in data for "required blocks"
                        blockDataList[i] = blockData;

                        _downloadBlockRemaining.decrement(segmentId);
                        debug(
                                "Read from local block for Segment ID = %" PRIu64 " Block ID = %" PRIu32 "\n",
                                segmentId, i);

                    } else {
                        // request block from other OSD
                        debug("sending request for block %" PRIu32 "\n", i);
                        _osdCommunicator->getBlockRequest(osdId, segmentId, i,
                                blockSymbols.second, DOWNLOAD, isParity);
                    }
                }

                // 4. wait until all blocks have arrived

                while (1) {
                    if (_downloadBlockRemaining.get(segmentId) == 0) {

						/*
                        for (BlockData block : blockDataList) {
                            unsigned char checksum[MD5_DIGEST_LENGTH];
                            memset (checksum, 0, MD5_DIGEST_LENGTH);
                            MD5((unsigned char*) block.buf, block.info.blockSize,
                                    checksum);
                            debug ("block ID = %" PRIu32 " checksum = %s\n", block.info.blockId, md5ToHex(checksum).c_str());
                        }
                        */

                        // 5. decode blocks

                        debug(
                                "[DOWNLOAD] Start Decoding with %d scheme and settings = %s\n",
                                (int )codingScheme, codingSetting.c_str());
                        segmentData = _codingModule->decodeBlockToSegment(
                                codingScheme, blockDataList,
                                requiredBlockSymbols, segmentSize,
                                codingSetting);

                        // clean up block data
                        _downloadBlockRemaining.erase(segmentId);

                        for (auto blockSymbols : requiredBlockSymbols) {
                            uint32_t i = blockSymbols.first;
                            debug(
                                    "%" PRIu32 " free block %" PRIu32 " addr = %p\n",
                                    i, blockDataList[i].info.blockId,
                                    blockDataList[i].buf);
                            MemoryPool::getInstance().poolFree(
                                    blockDataList[i].buf);
                            debug("%" PRIu32 " block %" PRIu32 " free-d\n", i,
                                    blockDataList[i].info.blockId);
                        }
                        _downloadBlockData.erase(segmentId);

                        break;
                    } else {
                        usleep(USLEEP_DURATION); // 0.01s
                    }
                }
            }

            debug("%s\n", "[DOWNLOAD] Send Segment");

            _isSegmentDownloaded.set(segmentId, true);
        } else {
#ifdef USE_SEGMENT_CACHE
            // hack: trigger MDS to update hotness
            _osdCommunicator->getSegmentInfoRequest(segmentId, _osdId, false);
#endif
        }
    }

    // 5. send segment if not localRetrieve
    if (!localRetrieve) {
        _osdCommunicator->sendSegment(_osdId, sockfd, segmentData);
    } else {
        cacheSegment(segmentId, segmentData);
        _osdCommunicator->replyCacheSegment(requestId, sockfd, segmentId,
                _osdId);
    }

    // 6. cache and free
    segmentRequestCountMutex.lock();
    bool isLocked = true;

    _segmentRequestCount.decrement(segmentId);
    if (_segmentRequestCount.get(segmentId) == 0) {
        _segmentRequestCount.erase(segmentId);

        // make a copy of segmentData and then erase
        SegmentData tempSegmentData = segmentData;
        _segmentDataMap.erase(segmentId);

        segmentRequestCountMutex.unlock();
        isLocked = false;

        freeSegment(segmentId, tempSegmentData);

        debug("%s\n", "[DOWNLOAD] Cleanup completed");
    }

    if (isLocked) {
        segmentRequestCountMutex.unlock();
    }

}

void Osd::getBlockRequestProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint32_t blockId, vector<offset_length_t> symbols,
        DataMsgType dataMsgType, bool isParity) {

    BlockData blockData;

    if (isParity) {
        #ifdef APPEND_PARITY
            #ifdef COMPACT_PARITY_ON_READ
            _storageModule->mergeBlock(segmentId, blockId, isParity);
            blockData = _storageModule->readBlock(
                    segmentId, blockId, symbols);
            #else
            blockData = _storageModule->getMergedBlock(segmentId, blockId, isParity);
            #endif
        #else
            blockData = _storageModule->readBlock(
                    segmentId, blockId, symbols);
        #endif
    } else {
        #ifdef APPEND_DATA
            blockData = _storageModule->getMergedBlock(segmentId, blockId, false);
        #else
            blockData = _storageModule->readBlock(
                    segmentId, blockId, symbols);
        #endif
    }

    _osdCommunicator->sendBlock(sockfd, blockData, dataMsgType);
    MemoryPool::getInstance().poolFree(blockData.buf);
    debug("Block ID = %" PRIu32 " free-d\n", blockId);
}

void Osd::retrieveRecoveryBlock(uint32_t recoverytpId, uint32_t osdId,
        uint64_t segmentId, uint32_t blockId,
        vector<offset_length_t> &offsetLength, BlockData &repairedBlock,
        bool isParity) {

    uint64_t recoveryLength = 0;
    for (auto it : offsetLength) {
        recoveryLength += it.second;
    }

    debug("[RECOVERY_DATA] Segment ID = %" PRIu64 " Length = %" PRIu64 "\n",
            segmentId, recoveryLength);

    if (osdId == _osdId) {
        // read block from disk

        if (isParity) {
            #ifdef APPEND_PARITY
                #ifdef COMPACT_PARITY_ON_READ
                _storageModule->mergeBlock(segmentId, blockId, true);
                repairedBlock = _storageModule->readBlock(
                        segmentId, blockId, offsetLength);
                #else
                repairedBlock = _storageModule->getMergedBlock(segmentId, blockId, true);
                #endif
            #else
                repairedBlock = _storageModule->readBlock(
                        segmentId, blockId, offsetLength);
            #endif
        } else {
            #ifdef APPEND_DATA
            repairedBlock = _storageModule->getMergedBlock(segmentId, blockId, false);
            #else
            repairedBlock = _storageModule->readBlock(
                        segmentId, blockId, offsetLength);
            #endif
        }
    } else {

        // create entry first, wait for putBlockInit to set real value
        const string blockKey = to_string(segmentId) + "." + to_string(blockId);

        // wait for recovery of the same block to complete
        while (_isPendingRecovery.count(blockKey)) {
            debug("NEED TO WAIT for %s\n", blockKey.c_str());
            usleep(USLEEP_DURATION);
        }

        _isPendingRecovery.set(blockKey, true);

        _osdCommunicator->getBlockRequest(osdId, segmentId, blockId,
                offsetLength, RECOVERY, isParity);
        debug_cyan("[RECOVERY] Requested Symbols for Block %" PRIu32 "\n",
                blockId);

        while (1) {
            if (_isPendingRecovery.get(blockKey) == false) {
                debug(
                        "[RECOVERY] retrieveRecoveryBlock returns for block %" PRIu64 ".%" PRIu32 "is received\n",
                        segmentId, blockId);
                break;
            } else {
                usleep(USLEEP_DURATION); // sleep 0.01s
            }
        }

        // retrieve block from _recoveryBlockData and cleanup
        repairedBlock = _recoveryBlockData.get(blockKey);
        _recoveryBlockData.erase(blockKey);
        _pendingRecoveryBlockChunk.erase(blockKey);
        _isPendingRecovery.erase(blockKey);
    }

    _recoverytpRequestCount.decrement(recoverytpId);
}

DataMsgType Osd::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint32_t segLength, uint32_t bufLength,
        uint32_t chunkCount, CodingScheme codingScheme, string setting,
        string checksum, string updateKey, bool isSmallSegment) {

    struct CodingSetting codingSetting;
    codingSetting.codingScheme = codingScheme;
    codingSetting.setting = setting;

    // save coding setting
    _codingSettingMap.set(segmentId, codingSetting);

    // determine dataMsgType
    SegmentTransferOsdInfo segmentInfo =
            _osdCommunicator->getSegmentInfoRequest(segmentId, _osdId);

    debug("SegmentInfo id = %" PRIu64 " size = %" PRIu32 "\n", segmentInfo._id,
            segmentInfo._size);

    DataMsgType dataMsgType = DEFAULT_DATA_MSG;
    if (segmentInfo._id == 0 || segmentInfo._size == 0) {
        dataMsgType = UPLOAD;
    } else {
        dataMsgType = UPDATE;
    }

    if (dataMsgType == UPLOAD) {
        // reduce memory consumption by limiting the number processing segments
        while (_pendingSegmentChunk.size() > MAX_NUM_PROCESSING_SEGMENT) {
            usleep(USLEEP_DURATION);
        }
        _pendingSegmentChunk.set(segmentId, chunkCount);
    } else if (dataMsgType == UPDATE) {
        _pendingUpdateSegmentChunk.set(updateKey, chunkCount);
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
        exit(-1);
    }

    // create segment and cache
    _storageModule->createSegmentCache(segmentId, segLength, bufLength,
            dataMsgType, updateKey);
    if (!isSmallSegment) {
        _osdCommunicator->replyPutSegmentInit(requestId, sockfd, segmentId,
                dataMsgType);
    }

#ifdef USE_CHECKSUM
    // save md5 to map
    _checksumMap.set(segmentId, checksum);
#endif

    return dataMsgType;
}

void Osd::distributeBlock(uint64_t segmentId, const struct BlockData blockData,
        const struct BlockLocation blockLocation, enum DataMsgType dataMsgType,
        uint32_t blocktpId) {
    debug("Distribute Block %" PRIu64 ".%" PRIu32 " to %" PRIu32 "\n",
            segmentId, blockLocation.blockId, blockLocation.osdId);
    // if destination is myself
    if (blockLocation.osdId == _osdId) {
        if (dataMsgType == UPLOAD) {
            _storageModule->createBlock(segmentId, blockData.info.blockId,
                    blockData.info.blockSize);
            _storageModule->writeBlock(segmentId, blockData.info.blockId,
                    blockData.buf, 0, blockData.info.blockSize);
            _storageModule->flushBlock(segmentId, blockData.info.blockId);
        } else if (dataMsgType == UPDATE) {
            debug("Updating Segment %" PRIu64 " Block %" PRIu32 "\n", segmentId,
                    blockData.info.blockId);

#ifdef APPEND_DATA
            uint32_t deltaId = _storageModule->getNextDeltaId(segmentId, blockData.info.blockId);
            _storageModule->createDeltaBlock(segmentId, blockData.info.blockId, deltaId);
            _storageModule->writeDeltaBlock(segmentId, blockData.info.blockId, deltaId, blockData.buf, blockData.info.offlenVector);
            _storageModule->flushDeltaBlock(segmentId, blockData.info.blockId, deltaId);
#else
            _storageModule->updateBlock(segmentId, blockData.info.blockId, blockData);
            _storageModule->flushBlock(segmentId, blockData.info.blockId);
#endif

        } else if (dataMsgType == PARITY) {
            debug("Updating Segment %" PRIu64 " Block %" PRIu32 "\n", segmentId,
                    blockData.info.blockId);

#ifdef APPEND_PARITY
            uint32_t deltaId = _storageModule->getNextDeltaId(segmentId, blockData.info.blockId);
            _storageModule->createDeltaBlock(segmentId, blockData.info.blockId, deltaId);
            _storageModule->writeDeltaBlock(segmentId, blockData.info.blockId, deltaId, blockData.buf, blockData.info.offlenVector);
            _storageModule->flushDeltaBlock(segmentId, blockData.info.blockId, deltaId);
#else
            _storageModule->updateBlock(segmentId, blockData.info.blockId, blockData);
            _storageModule->flushBlock(segmentId, blockData.info.blockId);
#endif

        }
    } else {
#ifdef MOUNT_OSD
        _storageModule->createRemoteBlock(blockLocation.osdId, segmentId,
                blockData.info.blockId, blockData.info.blockSize);
        _storageModule->writeRemoteBlock(blockLocation.osdId, segmentId,
                blockData.info.blockId, blockData.buf, 0,
                blockData.info.blockSize);
        _storageModule->flushRemoteBlock(blockLocation.osdId, segmentId,
                blockData.info.blockId);
#else
        uint32_t dstSockfd = _osdCommunicator->getSockfdFromId(
                blockLocation.osdId);
        if (dataMsgType == UPDATE || dataMsgType == PARITY) {
            uint32_t updateId = ++_updateId;
            string updateKey = to_string(_osdId) + "." + to_string(updateId);
            debug("updateKey = %s\n", updateKey.c_str());
            _osdCommunicator->sendBlock(dstSockfd, blockData, dataMsgType,
                    updateKey);
        } else {
            _osdCommunicator->sendBlock(dstSockfd, blockData, dataMsgType);
        }
#endif
    }

    // free memory
    MemoryPool::getInstance().poolFree(blockData.buf);

    if (blocktpId != 0) {
        _blocktpRequestCount.decrement(blocktpId);
    }

    debug(
            "Completed Distributing Block %" PRIu64 ".%" PRIu32 " to %" PRIu32 "\n",
            segmentId, blockData.info.blockId, blockLocation.osdId);

}

void Osd::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, DataMsgType dataMsgType, string updateKey,
        vector<offset_length_t> offsetLength, bool isSmallSegment) {

    if (dataMsgType != UPLOAD && dataMsgType != UPDATE) {
        debug_error("Invalid Message Type = %d\n", dataMsgType);
        exit(-1);
    }

    // TODO: check integrity of segment received
    while (1) {

        if ((dataMsgType == UPLOAD && _pendingSegmentChunk.get(segmentId) == 0)
                || (dataMsgType == UPDATE
                        && _pendingUpdateSegmentChunk.get(updateKey) == 0)) {
            // if all chunks have arrived
            struct SegmentData segmentCache = _storageModule->getSegmentData(
                    segmentId, dataMsgType, updateKey);

            unsigned char checksum[MD5_DIGEST_LENGTH];
            memset(checksum, 0, MD5_DIGEST_LENGTH);

#ifdef USE_CHECKSUM
            // compute md5 checksum
            MD5((unsigned char*) segmentCache.buf, segmentCache.segLength,
                    checksum);
            debug_cyan("md5 of segment ID %" PRIu64 " = %s\n",
                    segmentId, md5ToHex(checksum).c_str());

            // compare md5 with saved one
            if (_checksumMap.get(segmentId) != md5ToHex(checksum)) {
                debug_error("MD5 of Segment ID = %" PRIu64 " mismatch!\n",
                        segmentId);
                exit(-1);
            } else {
                debug("MD5 of Segment ID = %" PRIu64 " match\n", segmentId);
                _checksumMap.erase(segmentId);
            }
#endif

            struct CodingSetting codingSetting = _codingSettingMap.get(
                    segmentId);
            _codingSettingMap.erase(segmentId);

            debug("Segment Id = %" PRIu64 " Coding Scheme = %d setting = %s\n",
                    segmentId, (int ) codingSetting.codingScheme,
                    codingSetting.setting.c_str());

            // perform coding
            vector<struct BlockData> blockDataList;
            if (dataMsgType == UPLOAD) {
                blockDataList = _codingModule->encodeSegmentToBlock(
                        codingSetting.codingScheme, segmentId, segmentCache.buf,
                        segmentCache.info.segLength, codingSetting.setting);
            } else if (dataMsgType == UPDATE) {
                //... call codingModule to do update
                blockDataList = _codingModule->unpackUpdates(
                        codingSetting.codingScheme, segmentId, segmentCache.buf,
                        segmentCache.info.segLength, codingSetting.setting,
                        offsetLength);
            }

            // request secondary OSD list

            vector<struct BlockLocation> blockLocationList;
            if (dataMsgType == UPLOAD) {
                blockLocationList = _osdCommunicator->getOsdListRequest(
                        segmentId, MONITOR, blockDataList.size(), _osdId,
                        blockDataList[0].info.blockSize);
            } else if (dataMsgType == UPDATE) {
                // If it is update, append parity node info to secondary OSD
                SegmentTransferOsdInfo segmentInfo =
                        _osdCommunicator->getSegmentInfoRequest(segmentId,
                                _osdId);
                uint32_t blockNum = segmentInfo._osdList.size();
                uint32_t parityNum = _codingModule->getParityNumber(
                        codingSetting.codingScheme, codingSetting.setting);
                vector<BlockLocation> parityOsdListPair;
                for (uint32_t i = parityNum; i >= 1; --i) {
                    BlockLocation blockLocation;
                    blockLocation.blockId = blockNum - i;
                    blockLocation.osdId = segmentInfo._osdList[blockNum - i];
                    parityOsdListPair.push_back(blockLocation);
                }
                for (uint32_t i = 0; i < blockDataList.size(); i++) {
                    blockDataList[i].info.parityVector = parityOsdListPair;
                }

                // copy to blockLocationList
                for (uint32_t i = 0; i < segmentInfo._osdList.size(); i++) {
                    BlockLocation blockLocation;
                    blockLocation.blockId = i;
                    blockLocation.osdId = segmentInfo._osdList[i];
                    blockLocationList.push_back(blockLocation);
                }
            }

            vector<uint32_t> nodeList;

            uint32_t blocktpId = ++_blocktpId; // should not use 0
            _blocktpRequestCount.set(blocktpId, blockDataList.size());

            for (auto blockData : blockDataList) {
            
                // fill in codingScheme and codingSetting
                blockData.info.codingScheme = codingSetting.codingScheme;
                blockData.info.codingSetting = codingSetting.setting;
                blockData.info.segmentSize = segmentCache.info.segLength;

                // delta update case 1: primary OSD receives the update

                // if the update location is the primary OSD, just send the
                // delta to parity nodes
                if (dataMsgType == UPDATE
                        && blockLocationList[blockData.info.blockId].osdId
                                == _osdId) {
                    // send delta to parity nodes
                    sendDelta(segmentId, blockData.info.blockId, blockData,
                            blockData.info.offlenVector);
                }

#ifdef PARALLEL_TRANSFER
                debug("Thread Pool Status %d/%d/%d\n", (int )_blocktp.active(),
                        (int )_blocktp.pending(), (int )_blocktp.size());
                _blocktp.schedule(
                        boost::bind(&Osd::distributeBlock, this, segmentId,
                                blockData,
                                blockLocationList[blockData.info.blockId],
                                dataMsgType, blocktpId));
#else
                distributeBlock(segmentId, blockData,blockLocationList[blockData.info.blockId], dataMsgType);
#endif

                nodeList.push_back(
                        blockLocationList[blockData.info.blockId].osdId);

            }
#ifdef PARALLEL_TRANSFER
            // block until all blocks retrieved
            while (_blocktpRequestCount.get(blocktpId) > 0) {
                usleep(USLEEP_DURATION);
            }
#endif
            _blocktpRequestCount.erase(blocktpId);

            if (dataMsgType == UPLOAD) {
                _pendingSegmentChunk.erase(segmentId);
                // Acknowledge MDS for Segment Upload Completed
                _osdCommunicator->segmentUploadAck(segmentId,
                        segmentCache.info.segLength, codingSetting.codingScheme,
                        codingSetting.setting, nodeList, md5ToHex(checksum));
            } else {
                _pendingUpdateSegmentChunk.erase(updateKey);
            }

            cout << "Segment " << segmentId << " uploaded" << endl;

            // if all chunks have arrived, send ack
            _osdCommunicator->replyPutSegmentEnd(requestId, sockfd, segmentId, isSmallSegment);

#ifdef CACHE_AFTER_TRANSFER
            // after ack, write segment cache to disk (and close file)
            _storageModule->putSegmentToDiskCache(segmentId, segmentCache);
#endif

            _storageModule->closeSegmentData(segmentId, dataMsgType, updateKey);
            break;
        } else {
            usleep(USLEEP_DURATION); // sleep 0.01s
        }

    }

}

vector<BlockData> Osd::computeDelta(uint64_t segmentId, uint32_t blockId,
        BlockData newBlock, vector<offset_length_t> offsetLength, vector<uint32_t> parityVector) {

    BlockData oldBlock = _storageModule->readBlock(segmentId, blockId,
            offsetLength);

    sort (parityVector.begin(), parityVector.end());

    debug(
            "Delta codingScheme = %d, codingSetting = %s, parityVectorSize = %zu\n",
            newBlock.info.codingScheme, newBlock.info.codingSetting.c_str(),
            parityVector.size());

    CodingScheme codingScheme = newBlock.info.codingScheme;
    string codingSetting = newBlock.info.codingSetting;
    vector<BlockData> deltas = _codingModule->computeDelta(codingScheme,
            codingSetting, oldBlock, newBlock, offsetLength, parityVector);

    MemoryPool::getInstance().poolFree(oldBlock.buf);

    return deltas;
}

void Osd::sendDelta(uint64_t segmentId, uint32_t blockId, BlockData newBlock,
        vector<offset_length_t> offsetLength) {
        
    if (newBlock.info.parityVector.empty()) {
        return;
    }

    // transform parityVector pair to simple list of parity block ID
    vector<uint32_t> parityBlockIdList;
    std::transform(newBlock.info.parityVector.begin(),
            newBlock.info.parityVector.end(),
            std::back_inserter(parityBlockIdList),
            [](BlockLocation const& x) {return x.blockId;});

    // update delta (should be performed before updating the block in-place)
    vector<BlockData> deltas = computeDelta(segmentId, blockId, newBlock,
            offsetLength, parityBlockIdList);
    for (int i = 0; i < (int)newBlock.info.parityVector.size(); i++) {

        BlockLocation blockLocation;
        blockLocation.osdId = newBlock.info.parityVector[i].osdId;
        blockLocation.blockId = newBlock.info.parityVector[i].blockId;    // replaced

        debug ("PARITY ID = %" PRIu32 " OSD ID = %" PRIu32 "\n", blockLocation.blockId, blockLocation.osdId);

        BlockData &delta = deltas[i];
        debug ("XXXXXX delta segmentid = %" PRIu64 " blockId = %" PRIu32 "\n", delta.info.segmentId, delta.info.blockId); 

        delta.info.blockId = newBlock.info.parityVector[i].blockId;

        debug(
                "Send delta of segment %" PRIu64 " block %" PRIu32 " to OSD %" PRIu32 "\n",
                segmentId, blockLocation.blockId, blockLocation.osdId);

        uint32_t blocktpId = ++_blocktpId;
        _blocktp.schedule(
                boost::bind(&Osd::distributeBlock, this, segmentId, delta,
                        blockLocation, PARITY, blocktpId));
    }
}

void Osd::putBlockEndProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint32_t blockId, DataMsgType dataMsgType,
        string updateKey, vector<offset_length_t> offsetLength,
        vector<BlockLocation> parityList, CodingScheme codingScheme,
        string codingSetting, uint64_t segmentSize) {

    // TODO: check integrity of block received
    const string blockKey = to_string(segmentId) + "." + to_string(blockId);

    debug("dataMsgType = %d\n", dataMsgType);

    if (dataMsgType == RECOVERY) {
        while (1) {
            if (_pendingRecoveryBlockChunk.get(blockKey) == 0) {
                debug(
                        "[RECOVERY] all chunks for block %" PRIu64 ".%" PRIu32 "is received\n",
                        segmentId, blockId);
                _isPendingRecovery.set(blockKey, false);
                _osdCommunicator->replyPutBlockEnd(requestId, sockfd, segmentId,
                        blockId);
                break;
            } else {
                usleep(USLEEP_DURATION); // sleep 0.01s
            }
        }
    } else if (dataMsgType == UPDATE) {
        while (1) {
            if (_pendingUpdateBlockChunk.get(updateKey) == 0) {

                debug("[UPDATE] all chunks for updateKey %s is received\n",
                        updateKey.c_str());
                struct BlockData blockData = _updateBlockData.get(updateKey);
                blockData.info.offlenVector = offsetLength;
                blockData.info.parityVector = parityList;
                blockData.info.codingScheme = codingScheme;
                blockData.info.codingSetting = codingSetting;
                blockData.info.segmentSize = segmentSize;

                // delta update case 2: secondary OSD receives the update
                // send delta to parity nodes
                sendDelta(segmentId, blockId, blockData, offsetLength);

#ifdef APPEND_DATA
                uint32_t deltaId = _storageModule->getNextDeltaId(segmentId, blockId);
                _storageModule->createDeltaBlock(segmentId, blockId, deltaId);
                _storageModule->writeDeltaBlock(segmentId, blockId, deltaId, blockData.buf, blockData.info.offlenVector);
                _storageModule->flushDeltaBlock(segmentId, blockId, deltaId);
#else
                _storageModule->updateBlock(segmentId, blockId, blockData);
                _storageModule->flushBlock(segmentId, blockId);
#endif

                MemoryPool::getInstance().poolFree(blockData.buf);
                _updateBlockData.erase(updateKey);
                _pendingUpdateBlockChunk.erase(updateKey);
            }

            // if all chunks have arrived, send ack
            if (!_pendingUpdateBlockChunk.count(updateKey)) {
                _osdCommunicator->replyPutBlockEnd(requestId, sockfd, segmentId,
                        blockId);
                break;
            } else {
                usleep(USLEEP_DURATION);
            }
        }
    } else if (dataMsgType == PARITY) {
        while (1) {
            if (_pendingUpdateBlockChunk.get(updateKey) == 0) {
                debug("[PARITY] all chunks for updateKey %s is received\n",
                        updateKey.c_str());
                struct BlockData blockData = _updateBlockData.get(updateKey);
                blockData.info.offlenVector = offsetLength;
                blockData.info.parityVector = parityList;

#ifdef APPEND_PARITY
                uint32_t deltaId = _storageModule->getNextDeltaId(segmentId,
                        blockId);

                // e.g. if there is 3 delta [0 1 2], the deltaId for the new one is 3
                _storageModule->createDeltaBlock(segmentId, blockId, deltaId);
                _storageModule->writeDeltaBlock(segmentId, blockId, deltaId,
                        blockData.buf, offsetLength);
                _storageModule->flushDeltaBlock(segmentId, blockId, deltaId);
#else
                // read old parity and compute XOR
                uint32_t combinedLength = StorageModule::getCombinedLength(offsetLength);
                BlockData parityDelta = _storageModule->readBlock(segmentId, blockId,
                        offsetLength);
                Coding::bitwiseXor(parityDelta.buf, parityDelta.buf, blockData.buf, combinedLength);

                _storageModule->updateBlock(segmentId, blockId, parityDelta);
                _storageModule->flushBlock(segmentId, blockId);

                MemoryPool::getInstance().poolFree(parityDelta.buf);
#endif
                MemoryPool::getInstance().poolFree(blockData.buf);
                _updateBlockData.erase(updateKey);
                _pendingUpdateBlockChunk.erase(updateKey);
            }

            // if all chunks have arrived, send ack
            if (!_pendingUpdateBlockChunk.count(updateKey)) {
                _osdCommunicator->replyPutBlockEnd(requestId, sockfd, segmentId,
                        blockId);
                break;
            } else {
                usleep(USLEEP_DURATION);
            }
        }
    } else if (dataMsgType == DOWNLOAD || dataMsgType == UPLOAD) {
        while (1) {
            if (_pendingBlockChunk.get(blockKey) == 0) {
                if (dataMsgType == DOWNLOAD) {
                    // for download, do nothing, handled by getSegmentRequestProcessor
                    _downloadBlockRemaining.decrement(segmentId);
                    debug(
                            "[DOWNLOAD] all chunks for block %" PRIu32 "is received\n",
                            blockId);
                } else if (dataMsgType == UPLOAD) {
                    // write block in one go
                    struct BlockData blockData = _uploadBlockData.get(blockKey);

                    debug(
                            "[UPLOAD] all chunks for block %" PRIu32 "is received blockSize = %" PRIu32 "\n",
                            blockId, blockData.info.blockSize);

                    _storageModule->createBlock(segmentId, blockId,
                            blockData.info.blockSize);
                    _storageModule->writeBlock(segmentId, blockId,
                            blockData.buf, 0, blockData.info.blockSize);
                    _storageModule->flushBlock(segmentId, blockId);

                    MemoryPool::getInstance().poolFree(blockData.buf);
                    _uploadBlockData.erase(blockKey);
                }
                // remove from map
                _pendingBlockChunk.erase(blockKey);
            }
            // if all chunks have arrived, send ack
            if (!_pendingBlockChunk.count(blockKey)) {
                _osdCommunicator->replyPutBlockEnd(requestId, sockfd, segmentId,
                        blockId);
                break;
            } else {
                usleep(USLEEP_DURATION); // sleep 0.01s
            }
        }
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
    }
}

uint32_t Osd::putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint64_t offset, uint32_t length,
        DataMsgType dataMsgType, string updateKey, char* buf) {

    uint32_t byteWritten;
    byteWritten = _storageModule->writeSegmentData(segmentId, buf, offset,
            length, dataMsgType, updateKey);

    if (dataMsgType == UPLOAD) {
        _pendingSegmentChunk.decrement(segmentId);
    } else if (dataMsgType == UPDATE) {
        _pendingUpdateSegmentChunk.decrement(updateKey);
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
    }

    return byteWritten;
}

void Osd::putBlockInitProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint32_t blockId, uint32_t length,
        uint32_t chunkCount, DataMsgType dataMsgType, string updateKey) {

    const string blockKey = to_string(segmentId) + "." + to_string(blockId);

    debug(
            "[PUT_BLOCK_INIT] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 " DataMsgType = %d\n",
            segmentId, blockId, length, chunkCount, dataMsgType);

    if (dataMsgType == DOWNLOAD) {
        _pendingBlockChunk.set(blockKey, chunkCount);
        struct BlockData& blockData = _downloadBlockData.get(segmentId)[blockId];
        blockData.info.segmentId = segmentId;
        blockData.info.blockId = blockId;
        blockData.info.blockSize = length;
        blockData.buf = MemoryPool::getInstance().poolMalloc(length);
    } else {
        BlockData blockData;
        blockData.info.segmentId = segmentId;
        blockData.info.blockId = blockId;
        blockData.info.blockSize = length;
        blockData.buf = MemoryPool::getInstance().poolMalloc(length);
        if (dataMsgType == RECOVERY) {
            _pendingRecoveryBlockChunk.set(blockKey, chunkCount);
            _recoveryBlockData.set(blockKey, blockData);
        } else if (dataMsgType == UPLOAD) {
            _pendingBlockChunk.set(blockKey, chunkCount);
            _uploadBlockData.set(blockKey, blockData);
        } else if (dataMsgType == UPDATE || dataMsgType == PARITY) {
            _pendingUpdateBlockChunk.set(updateKey, chunkCount);
            _updateBlockData.set(updateKey, blockData);
        } else {
            debug_error("Invalid data message type = %d\n", dataMsgType);
            exit(-1);
        }
    }
    _osdCommunicator->replyPutBlockInit(requestId, sockfd, segmentId, blockId);
}

uint32_t Osd::putBlockDataProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, uint32_t blockId, uint32_t offset, uint32_t length,
        char* buf, DataMsgType dataMsgType, string updateKey) {

    const string blockKey = to_string(segmentId) + "." + to_string(blockId);

    uint32_t chunkLeft = 0;

    if (dataMsgType == RECOVERY) {
        struct BlockData& blockData = _recoveryBlockData.get(blockKey);
        memcpy(blockData.buf + offset, buf, length);
        chunkLeft = _pendingRecoveryBlockChunk.decrement(blockKey);
    } else if (dataMsgType == DOWNLOAD) {
        struct BlockData& blockData = _downloadBlockData.get(segmentId)[blockId];
        memcpy(blockData.buf + offset, buf, length);
        chunkLeft = _pendingBlockChunk.decrement(blockKey);
    } else if (dataMsgType == UPLOAD) {
        struct BlockData& blockData = _uploadBlockData.get(blockKey);
        memcpy(blockData.buf + offset, buf, length);
        chunkLeft = _pendingBlockChunk.decrement(blockKey);
    } else if (dataMsgType == UPDATE || dataMsgType == PARITY) {
        struct BlockData& blockData = _updateBlockData.get(updateKey);
        memcpy(blockData.buf + offset, buf, length);
        chunkLeft = _pendingUpdateBlockChunk.decrement(updateKey);
    } else {
        debug_error("Invalid data message type = %d\n", dataMsgType);
        exit(-1);
    }

    debug(
            "[BLOCK_DATA] dataMsgType = %s, segmentId = %" PRIu64 " blockId = %" PRIu32 " chunkLeft = %" PRIu32 "\n",
            EnumToString::toString(dataMsgType), segmentId, blockId, chunkLeft);

    return length;
}

void Osd::repairSegmentInfoProcessor(uint32_t requestId, uint32_t sockfd,
        uint64_t segmentId, vector<uint32_t> repairBlockList,
        vector<uint32_t> repairBlockOsdList) {

    string repairBlockOsdListString;
    for (uint32_t osd : repairBlockOsdList) {
        repairBlockOsdListString += to_string(osd) + " ";
    }
    debug_yellow(
            "Repair OSD List repairBlockList size = %zu Osd List size = %zu: %s\n",
            repairBlockList.size(), repairBlockOsdList.size(),
            repairBlockOsdListString.c_str());

    //    lock_guard<mutex> lk(recoveryMutex);

    // get coding information from MDS
    SegmentTransferOsdInfo segmentInfo =
            _osdCommunicator->getSegmentInfoRequest(segmentId, _osdId, true,
            true);

    const CodingScheme codingScheme = segmentInfo._codingScheme;
    const string codingSetting = segmentInfo._codingSetting;
    const uint32_t segmentSize = segmentInfo._size;

    debug_cyan("[RECOVERY] Coding Scheme = %d setting = %s\n",
            (int ) codingScheme, codingSetting.c_str());

    // get block status
    vector<bool> blockStatus = _osdCommunicator->getOsdStatusRequest(
            segmentInfo._osdList);

    // obtain required blockSymbols for repair
    block_list_t blockSymbols = _codingModule->getRepairBlockSymbols(
            codingScheme, repairBlockList, blockStatus, segmentSize,
            codingSetting);

    // obtain blocks from other OSD
    vector<BlockData> repairBlockData(
            _codingModule->getNumberOfBlocks(codingScheme, codingSetting));

    // initialize map for tracking recovery
    uint32_t recoverytpId = ++_recoverytpId; // should not use 0
    _recoverytpRequestCount.set(recoverytpId, blockSymbols.size());
    debug("blockSymbols.size = %zu\n", blockSymbols.size());

    for (auto block : blockSymbols) {

        uint32_t blockId = block.first;
        uint32_t osdId = segmentInfo._osdList[blockId];

        uint32_t parityCount = _codingModule->getParityNumber(codingScheme, codingSetting);
        uint32_t totalNumOfBlocks = _codingModule->getNumberOfBlocks(codingScheme, codingSetting);
        bool isParity = (blockId >= totalNumOfBlocks - parityCount);

        vector<offset_length_t> offsetLength = block.second;

        debug_cyan(
                "[RECOVERY] Need to obtain %zu symbols in block %" PRIu32 " from OSD %" PRIu32 "\n",
                offsetLength.size(), blockId, osdId);

        _recoverytp.schedule(
                boost::bind(&Osd::retrieveRecoveryBlock, this, recoverytpId,
                        osdId, segmentId, blockId, offsetLength,
                        boost::ref(repairBlockData[blockId]), isParity));

    }

    // block until all recovery blocks retrieved
    while (_recoverytpRequestCount.get(recoverytpId) > 0) {
        usleep(USLEEP_DURATION);
    }
    _recoverytpRequestCount.erase(requestId);

    debug_cyan(
            "[RECOVERY] Performing Repair for Segment %" PRIu64 " setting = %s\n",
            segmentId, codingSetting.c_str());

    // perform repair
    vector<BlockData> repairedBlocks = _codingModule->repairBlocks(codingScheme,
            repairBlockList, repairBlockData, blockSymbols, segmentSize,
            codingSetting);

    debug_cyan(
            "[RECOVERY] Distributing repaired blocks for segment %" PRIu64 "\n",
            segmentId);

    uint32_t j = 0;
    for (auto repairedBlock : repairedBlocks) {
        debug(
                "[RECOVERED_BLOCK] Segment ID = %" PRIu64 " Block ID = %" PRIu32 " codingScheme = %d Length = %" PRIu32 "\n",
                segmentId, repairedBlock.info.blockId, (int )codingScheme,
                repairedBlock.info.blockSize);
        BlockLocation blockLocation;
        blockLocation.blockId = repairedBlock.info.blockId;
        blockLocation.osdId = repairBlockOsdList[j];
        distributeBlock(segmentId, repairedBlock, blockLocation, UPLOAD); // free-d here
        j++;
    }

    // cleanup
    for (auto block : blockSymbols) {
        uint32_t blockId = block.first;
        MemoryPool::getInstance().poolFree(repairBlockData[blockId].buf);
    }

    // TODO: repairBlockOsd fails at this point?
    // send success message to MDS
    _osdCommunicator->repairBlockAck(segmentId, repairBlockList,
            repairBlockOsdList);

    debug("[RECOVERY] Recovery completed for segment %" PRIu64 "\n", segmentId);
}

void Osd::OsdStatUpdateRequestProcessor(uint32_t requestId, uint32_t sockfd) {
    OsdStatUpdateReplyMsg* replyMsg = new OsdStatUpdateReplyMsg(
            _osdCommunicator, sockfd, _osdId, getFreespace(), getCpuLoadavg(2));
    replyMsg->prepareProtocolMsg();
    _osdCommunicator->addMessage(replyMsg);
}

void Osd::NewOsdRegisterProcessor(uint32_t requestId, uint32_t sockfd,
        uint32_t osdId, uint32_t osdIp, uint32_t osdPort) {
    if (_osdId > osdId) {
        // Do connect
        _osdCommunicator->connectToOsd(osdIp, osdPort);
    }
}

void Osd::OnlineOsdListProcessor(uint32_t requestId, uint32_t sockfd,
        vector<struct OnlineOsd>& onlineOsdList) {

    for (uint32_t i = 0; i < onlineOsdList.size(); ++i) {
        if (_osdId > onlineOsdList[i].osdId) {
            // Do connect
            _osdCommunicator->connectToOsd(onlineOsdList[i].osdIp,
                    onlineOsdList[i].osdPort);

        }
    }
}

uint32_t Osd::getCpuLoadavg(int idx) {
    double load[3];
    int ret = getloadavg(load, 3);
    if (ret < idx) {
        return (INF);
    } else {
        return ((uint32_t) (load[idx] * 100));
    }
}

uint32_t Osd::getFreespace() {
    struct statvfs64 fiData;
    if ((statvfs64(DISK_PATH, &fiData)) < 0) {
        printf("Failed to stat %s:\n", DISK_PATH);
        return 0;
    } else {
        return ((uint32_t) (fiData.f_bsize * fiData.f_bfree / 1024 / 1024));
    }
}

OsdCommunicator * Osd::getCommunicator() {
    return _osdCommunicator;
}

StorageModule * Osd::getStorageModule() {
    return _storageModule;
}

uint32_t Osd::getOsdId() {
    return _osdId;
}
