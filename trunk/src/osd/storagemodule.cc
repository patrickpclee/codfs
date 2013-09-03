/*
 * storagemodule.cc
 */

#include <linux/falloc.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "storagemodule.hh"
#include "../common/debug.hh"
#include "../common/define.hh"
#include "../common/convertor.hh"
#include "../common/deltalocation.hh"
#include "../coding/coding.hh"

// global variable defined in each component
extern ConfigLayer* configLayer;

mutex fileMutex[2];
mutex uploadCacheMutex;
mutex updateCacheMutex;
mutex diskCacheMutex;

#ifdef USE_IO_THREADS
using namespace boost::threadpool;
#endif


StorageModule::StorageModule() {
    _openedFile = new FileLruCache<string, FILE*>(MAX_OPEN_FILES);
    _segmentUploadCache = {};
    _segmentUpdateCache = {};
    _segmentFolder = configLayer->getConfigString(
            "Storage>SegmentCacheLocation");
    _blockFolder = configLayer->getConfigString("Storage>BlockLocation");

#ifdef USE_IO_THREADS
    // create threadpool
    _iotp.size_controller().resize(IO_THREADS);
#endif

    // create folder if not exist
    struct stat st;
    if (stat(_segmentFolder.c_str(), &st) != 0) {
        debug("%s does not exist, make directory automatically\n",
                _segmentFolder.c_str());
        if (mkdir(_segmentFolder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
            perror("mkdir");
            exit(-1);
        }
    }
    if (stat(_blockFolder.c_str(), &st) != 0) {
        debug("%s does not exist, make directory automatically\n",
                _blockFolder.c_str());
        if (mkdir(_blockFolder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
            perror("mkdir");
            exit(-1);
        }
    }

    // Unit in StorageModule: Bytes
    _maxSegmentCache = stringToByte(
            configLayer->getConfigString("Storage>SegmentCacheCapacity"));
    _maxBlockCapacity = stringToByte(
            configLayer->getConfigString("Storage>BlockCapacity"));

    cout << "=== STORAGE ===" << endl;
    cout << "Segment Cache Location = " << _segmentFolder << " Size = "
            << formatSize(_maxSegmentCache) << endl;
    cout << "Block Storage Location = " << _blockFolder << " Size = "
            << formatSize(_maxBlockCapacity) << endl;
    cout << "===============" << endl;

    initializeStorageStatus();
}

StorageModule::~StorageModule() {

}

void StorageModule::initializeStorageStatus() {

    //
    // initialize segments
    //

    _freeSegmentSpace = _maxSegmentCache;
    _freeBlockSpace = _maxBlockCapacity;
    _currentSegmentUsage = 0;
    _currentBlockUsage = 0;

    struct dirent* dent;
    DIR* srcdir;

    srcdir = opendir(_segmentFolder.c_str());
    if (srcdir == NULL) {
        perror("opendir");
        exit(-1);
    }

    cout << "===== List of Files =====" << endl;

    while ((dent = readdir(srcdir)) != NULL) {
        struct stat st;

        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
            perror(dent->d_name);
            continue;
        }

        // save file info
        struct SegmentDiskCache segmentDiskCache;
        uint64_t segmentId = boost::lexical_cast<uint64_t>(dent->d_name);
        segmentDiskCache.length = st.st_size;
        segmentDiskCache.lastAccessedTime = st.st_atim;
        segmentDiskCache.filepath = _segmentFolder + dent->d_name;

        {
            lock_guard<mutex> lk(diskCacheMutex);
            _segmentDiskCacheMap.set(segmentId, segmentDiskCache);
            _segmentDiskCacheQueue.push_back(segmentId);

            _freeSegmentSpace -= st.st_size;
            _currentSegmentUsage += st.st_size;
        }

        cout << "ID: " << segmentId << "\tLength: " << segmentDiskCache.length
                << "\t Modified: " << segmentDiskCache.lastAccessedTime.tv_sec
                << endl;

    }
    closedir(srcdir);

    cout << "=======================" << endl;

    cout << "Segment Cache Usage: " << formatSize(_currentSegmentUsage) << "/"
            << formatSize(_maxSegmentCache) << endl;

    //
    // initialize blocks
    //

    srcdir = opendir(_blockFolder.c_str());
    if (srcdir == NULL) {
        perror("opendir");
        exit(-1);
    }

    while ((dent = readdir(srcdir)) != NULL) {
        struct stat st;

        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
            perror(dent->d_name);
            continue;
        }

        // save file info
        _freeBlockSpace -= st.st_size;
        _currentBlockUsage += st.st_size;

    }
    closedir(srcdir);

    cout << "Block Storage Usage: " << formatSize(_currentBlockUsage) << "/"
            << formatSize(_maxBlockCapacity) << endl;

}

uint64_t StorageModule::getFilesize(string filepath) {

    struct stat st;
    stat(filepath.c_str(), &st);
    return (uint64_t) st.st_size;

}

void StorageModule::createSegmentCache(uint64_t segmentId, uint32_t segLength,
        uint32_t bufLength, DataMsgType dataMsgType, string updateKey) {

    // create cache
    struct SegmentData segmentCache;
    segmentCache.info.segmentId = segmentId;
    segmentCache.bufLength = bufLength;
    segmentCache.info.segLength = segLength;
    //segmentCache.buf = MemoryPool::getInstance().poolMalloc(bufLength);
    segmentCache.buf = MemoryPool::getInstance().poolMalloc(segLength);

    // save cache to map
    if (dataMsgType == UPLOAD) {
        {
            lock_guard<mutex> lk(uploadCacheMutex);
            _segmentUploadCache[segmentId] = segmentCache;
        }
        debug("SegmentData created ID = %" PRIu64 " Length = %" PRIu32 "\n",
                segmentId, bufLength);
    } else if (dataMsgType == UPDATE) {
        {
            lock_guard<mutex> lk(updateCacheMutex);
            _segmentUpdateCache[updateKey] = segmentCache;
        }
        debug(
                "SegmentUpdateTransferCache created ID = %" PRIu64 " Length = %" PRIu32 "key = %s\n",
                segmentId, bufLength, updateKey.c_str());
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
        exit(-1);
    }
    debug(
            "SegmentData created ID = %" PRIu64 " segLength = %" PRIu32 " bufLength = %" PRIu32 "\n",
            segmentId, segLength, bufLength);
}

void StorageModule::createBlock(uint64_t segmentId, uint32_t blockId,
        uint32_t length) {

    const string blockKey = getBlockKey (segmentId, blockId);

    const string filepath = generateBlockPath(segmentId, blockId, _blockFolder);

    debug(
            "Block created ObjID = %" PRIu64 " BlockID = %" PRIu32 " Length = %" PRIu32 " Path = %s\n",
            segmentId, blockId, length, filepath.c_str());

    // initialize delta information
    {
        RWMutex* rwmutex = obtainRWMutex(blockKey);
        writeLock wtlock(*rwmutex);

        createFile(filepath);

        _deltaIdMap[blockKey] = 0;
        _deltaLocationMap[blockKey] = vector<DeltaLocation>();
        ReserveSpaceInfo reserveSpaceInfo;
        reserveSpaceInfo.currentOffset = length;
        reserveSpaceInfo.remainingReserveSpace = 0;
        reserveSpaceInfo.blockSize = length;
        _reserveSpaceMap[blockKey] = reserveSpaceInfo;
    }
}

void StorageModule::reserveBlockSpace(uint64_t segmentId, uint32_t blockId,
        uint32_t offset, uint32_t reserveLength) {

    const string blockKey = getBlockKey (segmentId, blockId);

    const string filepath = generateBlockPath(segmentId, blockId, _blockFolder);

    FILE* ptr = openFile(filepath);

    {
        RWMutex* rwmutex = obtainRWMutex(blockKey);
        writeLock wtlock(*rwmutex);

        if (fallocate (fileno (ptr), 0, offset, reserveLength) < 0) {
            debug_error ("Failed to reserve space: %s\n", filepath.c_str());
            exit (-1);
        }

        ReserveSpaceInfo reserveSpaceInfo;
        reserveSpaceInfo.currentOffset = offset;
        reserveSpaceInfo.remainingReserveSpace = reserveLength;
        reserveSpaceInfo.blockSize = offset;
        _reserveSpaceMap[blockKey] = reserveSpaceInfo;
    }
}

void StorageModule::createDeltaBlock(uint64_t segmentId, uint32_t blockId,
        uint32_t deltaId, bool useReserve) {

    // if trying to put it in reserve, do nothing right now
    if (useReserve) {
        return;
    }

    const string filepath = generateDeltaBlockPath(segmentId, blockId, deltaId,
            _blockFolder);
    createFile(filepath);

    debug(
            "Delta Block created ObjID = %" PRIu64 " BlockID = %" PRIu32 " DeltaID = %" PRIu32 " Path = %s\n",
            segmentId, blockId, deltaId, filepath.c_str());
}

uint32_t StorageModule::getDeltaCount (uint32_t segmentId, uint32_t blockId) {
    const string blockKey = getBlockKey (segmentId, blockId);
    // lock by the caller
    return _deltaLocationMap.get(blockKey).size(); // this is the real count of written delta
}

uint32_t StorageModule::getNextDeltaId (uint32_t segmentId, uint32_t blockId) {
    const string blockKey = getBlockKey (segmentId, blockId);
    RWMutex* rwmutex = obtainRWMutex(blockKey);
    writeLock wtlock(*rwmutex);
    uint32_t curDeltaId = _deltaIdMap[blockKey];
    _deltaIdMap[blockKey]++;
    return curDeltaId;
}

bool StorageModule::isSegmentCached(uint64_t segmentId) {

#ifndef USE_SEGMENT_CACHE
    return false;
#endif

    lock_guard<mutex> lk(diskCacheMutex);
    if (_segmentDiskCacheMap.count(segmentId)) {

        _segmentDiskCacheMap.get(segmentId).lastAccessedTime = {time(NULL), 0};

        _segmentDiskCacheQueue.remove(segmentId);
        _segmentDiskCacheQueue.push_back(segmentId);

        return true;
    }
    return false;
}

/**
 * Default length = 0 (read whole segment)
 */

struct SegmentData StorageModule::readSegment(uint64_t segmentId,
        uint64_t offsetInSegment, uint32_t length) {

    struct SegmentData segmentData;
    segmentData.info = readSegmentInfo(segmentId);

    // check num of bytes to read
    // if length = 0, read whole segment
    uint32_t byteToRead;
    if (length == 0) {
        byteToRead = segmentData.info.segLength;
    } else {
        byteToRead = length;
    }

    // TODO: check maximum malloc size
    // poolFree in osd_communicator::sendSegment
    segmentData.buf = MemoryPool::getInstance().poolMalloc(byteToRead);

    debug(
            "READ SEGMENT Segment ID = %" PRIu64 " read %" PRIu32 " bytes at offset %" PRIu64 "\n",
            segmentId, byteToRead, offsetInSegment);

    readFile(segmentData.info.segmentPath, segmentData.buf, offsetInSegment,
            byteToRead, true);

    debug(
            "Segment ID = %" PRIu64 " read %" PRIu32 " bytes at offset %" PRIu64 "\n",
            segmentId, byteToRead, offsetInSegment);

    return segmentData;

}

struct BlockData StorageModule::readBlock(uint64_t segmentId, uint32_t blockId,
        vector<offset_length_t> symbols) {

    uint32_t combinedLength = getCombinedLength(symbols);

    struct BlockData blockData;
    string blockPath = generateBlockPath(segmentId, blockId, _blockFolder);
    blockData.info.segmentId = segmentId;
    blockData.info.blockId = blockId;
    blockData.info.blockSize = combinedLength;
    blockData.info.offlenVector = symbols;
    blockData.buf = MemoryPool::getInstance().poolMalloc(combinedLength);
    char* bufptr = blockData.buf;

    //uint32_t blockOffset = 0;
    for (auto offsetLengthPair : symbols) {
        uint32_t offset = offsetLengthPair.first;
        uint32_t length = offsetLengthPair.second;
        debug(
                "READ BLOCK Symbol %s offset = %" PRIu32 " length = %" PRIu32 "\n",
                blockPath.c_str(), offset, length);
        readFile(blockPath, bufptr, offset, length, false);
        bufptr += length;
    }

    debug("Segment ID = %" PRIu64 " Block ID = %" PRIu32 " read %zu symbols\n",
            segmentId, blockId, symbols.size());

    return blockData;

}

// this function is not thread-safe, make sure the caller has lock
BlockData StorageModule::readDeltaFromReserve(uint64_t segmentId,
        uint32_t blockId, uint32_t deltaId, DeltaLocation deltaLocation) {
    BlockData blockData = doReadDelta (segmentId, blockId, deltaId, true, deltaLocation.offsetLength.first);
    return blockData;

}

// this function is not thread-safe, make sure the caller has lock
BlockData StorageModule::readDeltaBlock(uint64_t segmentId,
        uint32_t blockId, uint32_t deltaId) {
    BlockData blockData = doReadDelta (segmentId, blockId, deltaId, false, 0);
    return blockData;
}

// this function is not thread-safe, make sure the caller has lock
BlockData StorageModule::doReadDelta(uint64_t segmentId, uint32_t blockId,
        uint32_t deltaId, bool isReserve, uint32_t offset) {

    string blockKey = getBlockKey(segmentId, blockId);

    string blockPath;
    if (isReserve) {
        blockPath = generateBlockPath(segmentId, blockId, _blockFolder);
    } else {
        blockPath = generateDeltaBlockPath(segmentId, blockId, deltaId,
                _blockFolder);
    }

    debug ("Read delta from blockPath = %s\n", blockPath.c_str());

    string deltaKey = generateDeltaKey (segmentId, blockId, deltaId);
    vector<offset_length_t> offsetLength = _deltaOffsetLength[deltaKey];
    uint32_t combinedLength = getCombinedLength(offsetLength);

    debug ("Read delta length = %" PRIu32 " offsetLength size = %zu\n", combinedLength, offsetLength.size());

    struct BlockData blockData;
    blockData.info.segmentId = segmentId;
    blockData.info.blockId = blockId;
    blockData.info.blockSize = combinedLength; // size of delta
    blockData.info.offlenVector = offsetLength;
    blockData.buf = MemoryPool::getInstance().poolMalloc(combinedLength);

    // read whole delta block into memory
    readFile(blockPath, blockData.buf, offset, combinedLength, false);

    return blockData;
}

// this function is only thread-safe when needLock == true
BlockData StorageModule::getMergedBlock (uint64_t segmentId, uint32_t blockId, bool isParity, bool needLock) {

    const string blockPath = generateBlockPath(segmentId, blockId, _blockFolder);
    const string blockKey = getBlockKey (segmentId, blockId);

    RWMutex* rwmutex = obtainRWMutex(blockKey);

    readLock rdlock(*rwmutex, boost::try_to_lock_t());

    // if not need lock, it means the caller already has lock
    if (needLock && !rdlock.owns_lock()) {
        rdlock.lock();
    }

    uint32_t deltaCount = getDeltaCount(segmentId, blockId);

    // read whole block into memory
    BlockData blockData;
    blockData.info.segmentId = segmentId;
    blockData.info.blockId = blockId;
    blockData.info.blockType = (BlockType) isParity;
    blockData.info.blockSize = _reserveSpaceMap.get(blockKey).blockSize;
    blockData.info.offlenVector.push_back(make_pair (0, blockData.info.blockSize)); // merged

    debug(
            "Getting merged block for segment ID %" PRIu64 " block ID %" PRIu32 " isParity = %d blockSize = %" PRIu32 " deltaCount = %" PRIu32 "\n",
            segmentId, blockId, isParity, blockData.info.blockSize, deltaCount);

    // read block + reserve to wholeBuf
    const uint32_t byteToRead = _reserveSpaceMap.get(blockKey).currentOffset;
    char* wholeBuf = MemoryPool::getInstance().poolMalloc(byteToRead);
    readFile(blockPath, wholeBuf, 0, byteToRead, false);

    // copy to blockData part
    blockData.buf = MemoryPool::getInstance().poolMalloc(blockData.info.blockSize);
    memcpy (blockData.buf, wholeBuf, blockData.info.blockSize);

    // for each delta block, merge into parity for each <offset, length> using XOR
    for (DeltaLocation deltaLocation : _deltaLocationMap.get(blockKey)) {
        const uint32_t deltaId = deltaLocation.deltaId;

        // read from reserved space if delta is inside
        BlockData delta;
        if (deltaLocation.isReserveSpace) {
            debug ("Reading from Reserve Segment ID = %" PRIu64 " Block ID = %" PRIu32 " Delta ID = %" PRIu32 "\n", segmentId, blockId, deltaId);
            string deltaKey = generateDeltaKey (segmentId, blockId, deltaId);
            vector<offset_length_t> offsetLength = _deltaOffsetLength[deltaKey];
            uint32_t combinedLength = getCombinedLength(offsetLength);
            delta.info.segmentId = segmentId;
            delta.info.blockId = blockId;
            delta.info.blockSize = combinedLength; // size of delta
            delta.info.offlenVector = offsetLength;
            delta.buf = MemoryPool::getInstance().poolMalloc(combinedLength);
            memcpy (delta.buf, wholeBuf + deltaLocation.offsetLength.first, combinedLength);
        } else {
            debug ("Reading from Delta Block Segment ID = %" PRIu64 " Block ID = %" PRIu32 " Delta ID = %" PRIu32 "\n", segmentId, blockId, deltaId);
            delta = readDeltaBlock(segmentId, blockId, deltaId);
        }

        // perform merging
        char* deltaBufPtr = delta.buf;
        for (offset_length_t offsetLengthPair : delta.info.offlenVector) {
            uint32_t offset = offsetLengthPair.first;
            uint32_t length = offsetLengthPair.second;
            debug("Merging Delta offset = %" PRIu32 " length = %" PRIu32 "\n", offset, length);
            if (isParity) {
                Coding::bitwiseXor(blockData.buf + offset, blockData.buf + offset, deltaBufPtr, length);
            } else {
                memcpy (blockData.buf + offset, deltaBufPtr, length);
            }
            deltaBufPtr += length;
        }
        MemoryPool::getInstance().poolFree(delta.buf);
    }
    MemoryPool::getInstance().poolFree(wholeBuf);
    return blockData;
}

// this function is not thread-safe, make sure the caller has lock
void StorageModule::mergeBlock (uint64_t segmentId, uint32_t blockId, bool isParity) {

    const string blockKey = getBlockKey (segmentId, blockId);
    const string blockPath = generateBlockPath(segmentId, blockId, _blockFolder);
    uint32_t deltaCount = getDeltaCount(segmentId, blockId);
    debug ("Merge Block deltaCount = %" PRIu32 "\n", deltaCount);
    if (deltaCount == 0) {
        return;
    }

    // get the merged block
    BlockData blockData = getMergedBlock(segmentId, blockId, isParity, false);

    // save the whole merged parity into disk
    updateBlock(segmentId, blockId, blockData);
    tryCloseFile(blockPath);

    MemoryPool::getInstance().poolFree(blockData.buf);

    // remove deltas which are not in reserve
    for (DeltaLocation deltaLocation : _deltaLocationMap.get(blockKey)) {
        if (!deltaLocation.isReserveSpace) {
            const string deltaBlockPath = generateDeltaBlockPath(segmentId, blockId, deltaLocation.deltaId,
                    _blockFolder);

            // remove delta from disk
            tryCloseFile(deltaBlockPath);
            remove(deltaBlockPath.c_str());
        }
    }

    // remove all delta information
    _deltaLocationMap.get(blockKey).clear();
    _reserveSpaceMap.get(blockKey).remainingReserveSpace = RESERVE_SPACE_SIZE;
    _reserveSpaceMap.get(blockKey).currentOffset = blockData.info.blockSize;
}

uint32_t StorageModule::writeSegmentData(uint64_t segmentId, char* buf,
        uint64_t offsetInSegment, uint32_t length, DataMsgType dataMsgType,
        string updateKey) {

    char* recvCache;

    if (dataMsgType == UPLOAD) {
        {
            lock_guard<mutex> lk(uploadCacheMutex);
            if (!_segmentUploadCache.count(segmentId)) {
                debug_error("Cannot find cache for segment %" PRIu64 "\n",
                        segmentId);
                exit(-1);
            }
            recvCache = _segmentUploadCache[segmentId].buf;
        }
    } else if (dataMsgType == UPDATE) {
        {
            lock_guard<mutex> lk(updateCacheMutex);
            if (!_segmentUpdateCache.count(updateKey)) {
                debug_error("Cannot find cache for segment %" PRIu64 "\n",
                        segmentId);
                exit(-1);
            }
            recvCache = _segmentUpdateCache[updateKey].buf;
        }
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
        exit(-1);
    }

    memcpy(recvCache + offsetInSegment, buf, length);

    return length;
}

uint32_t StorageModule::writeSegmentDiskCache(uint64_t segmentId, char* buf,
        uint64_t offsetInSegment, uint32_t length) {

    uint32_t byteWritten;

    string filepath = generateSegmentPath(segmentId, _segmentFolder);

    // lower priority then other read / write
    byteWritten = writeFile(filepath, buf, offsetInSegment, length, true, 0);

    debug(
            "Segment ID = %" PRIu64 " write %" PRIu32 " bytes at offset %" PRIu64 "\n",
            segmentId, byteWritten, offsetInSegment);

    return byteWritten;
}

uint32_t StorageModule::writeBlock(uint64_t segmentId, uint32_t blockId,
        char* buf, uint64_t offsetInBlock, uint32_t length) {

    const string blockKey = getBlockKey (segmentId, blockId);
    RWMutex* rwmutex = obtainRWMutex(blockKey);
    writeLock wtlock(*rwmutex);

    uint32_t byteWritten = 0;

    string filepath = generateBlockPath(segmentId, blockId, _blockFolder);
    byteWritten = writeFile(filepath, buf, offsetInBlock, length, false);

    debug(
            "Segment ID = %" PRIu64 " Block ID = %" PRIu32 " write %" PRIu32 " bytes at offset %" PRIu64 "\n",
            segmentId, blockId, byteWritten, offsetInBlock);

    updateBlockFreespace(length);

    return byteWritten;
}

uint32_t StorageModule::writeDeltaBlock(uint64_t segmentId, uint32_t blockId,
        uint32_t deltaId, char* buf, vector<offset_length_t> offsetLength, bool isParity) {

    const string blockKey = getBlockKey (segmentId, blockId);
    RWMutex* rwmutex = obtainRWMutex(blockKey);
    writeLock wtlock(*rwmutex);

    /*
     *  +-----------------------+
     *  | PARITY BLOCK | AAAABB |
     *  +-----------------------+
     *  offsetLength means the original position in the parity block
     *  e.g., the first offset means the position where AAAA should be
     *  written during a merge
     *
     *  the block size means the actual compressed size of the delta
     *  e.g. the block size here is size of [AAAABB]
     */

    const string deltaKey = generateDeltaKey (segmentId, blockId, deltaId);
    const uint32_t combinedLength = getCombinedLength(offsetLength);

    ReserveSpaceInfo &reserveSpaceInfo = _reserveSpaceMap[blockKey];

    DeltaLocation deltaLocation;
    deltaLocation.blockId = blockId;
    deltaLocation.deltaId = deltaId;

    uint32_t currentOffset = 0;
    string filepath = "";
    if (isParity && combinedLength <= RESERVE_SPACE_SIZE) {
        if (reserveSpaceInfo.remainingReserveSpace < combinedLength) {
            // merge existing block and write again
            debug ("need merge remaining = %" PRIu32 " length = %" PRIu32 " deltaId = %" PRIu32 "\n", reserveSpaceInfo.remainingReserveSpace, combinedLength, deltaId);
            mergeBlock(segmentId, blockId, true);
        }

        currentOffset = reserveSpaceInfo.currentOffset;
        filepath = generateBlockPath(segmentId, blockId, _blockFolder);
        deltaLocation.offsetLength = make_pair(currentOffset, combinedLength);
        deltaLocation.isReserveSpace = true;
        debug ("block %" PRIu32 " written to reserve\n", blockId);
    } else {    // data block or delta too large
        createDeltaBlock(segmentId, blockId, deltaId, false);
        currentOffset = 0;
        filepath = generateDeltaBlockPath(segmentId, blockId, deltaId,
                _blockFolder);
        deltaLocation.offsetLength = make_pair(0, combinedLength);
        deltaLocation.isReserveSpace = false;
        debug ("block %" PRIu32 " written to delta\n", blockId);
    }

    uint32_t byteWritten = 0;

    // write delta block
    byteWritten = writeFile(filepath, buf, currentOffset, combinedLength, false);

    debug(
            "Segment ID = %" PRIu64 " Block ID = %" PRIu32 " Delta ID = %" PRIu32 " write %" PRIu32 " bytes\n",
            segmentId, blockId, deltaId, byteWritten);

    updateBlockFreespace(combinedLength);

    // if stored in reserve space
    if (deltaLocation.isReserveSpace) {
        reserveSpaceInfo.currentOffset += combinedLength;
        reserveSpaceInfo.remainingReserveSpace -= combinedLength;
    }

    _deltaOffsetLength[deltaKey] = offsetLength;
    _deltaLocationMap.get(blockKey).push_back(deltaLocation);

    return byteWritten;
}

uint32_t StorageModule::updateBlock(uint64_t segmentId, uint32_t blockId,
        BlockData blockData) {

    uint32_t byteWritten = 0;
    uint32_t curOffset = 0;

    string filepath = generateBlockPath(segmentId, blockId, _blockFolder);

    for (offset_length_t offsetLength : blockData.info.offlenVector) {
        debug("Update block offset %" PRIu32 " length %" PRIu32 "\n",
                offsetLength.first, offsetLength.second);
        byteWritten += writeFile(filepath, blockData.buf + curOffset,
                offsetLength.first, offsetLength.second, false);
        curOffset += offsetLength.second;
    }
    return byteWritten;
}

void StorageModule::closeSegmentData(uint64_t segmentId,
        DataMsgType dataMsgType, string updateKey) {

    if (dataMsgType == UPLOAD) {
        // close cache
        struct SegmentData segmentCache = getSegmentData(segmentId,
                dataMsgType);
        MemoryPool::getInstance().poolFree(segmentCache.buf);

        {
            lock_guard<mutex> lk(uploadCacheMutex);
            _segmentUploadCache.erase(segmentId);
        }
    } else if (dataMsgType == UPDATE) {
        // close cache
        struct SegmentData segmentCache = getSegmentData(segmentId, dataMsgType,
                updateKey);
        MemoryPool::getInstance().poolFree(segmentCache.buf);

        {
            lock_guard<mutex> lk(updateCacheMutex);
            _segmentUpdateCache.erase(updateKey);
        }
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
        exit(-1);
    }

    debug("Segment Cache ID = %" PRIu64 " closed\n", segmentId);
}

void StorageModule::doFlushFile(string filePath, bool &isFinished) {
    FILE* filePtr = NULL;
    try {
        filePtr = _openedFile->get(filePath);
        fflush(filePtr);
        //fsync(fileno(filePtr));
    } catch (out_of_range& oor) { // file pointer not found in cache
        return; // file already closed by cache, do nothing
    }
    isFinished = true;
    return;
}

void StorageModule::flushFile(string filePath) {
    bool isFinished = false;

#ifdef USE_IO_THREADS
    int priority = 10;
    schedule(_iotp,
            prio_task_func(priority,
                    boost::bind(&StorageModule::doFlushFile, this, filePath, boost::ref(isFinished))));

    while (!isFinished) {
        usleep(IO_POLL_INTERVAL);
    }

    return;
#else
    return doFlushFile(filePath, isFinished);
#endif
}

void StorageModule::flushSegmentDiskCache(uint64_t segmentId) {

    string filepath = generateSegmentPath(segmentId, _segmentFolder);
    flushFile(filepath);

}

void StorageModule::flushBlock(uint64_t segmentId, uint32_t blockId) {

    string filepath = generateBlockPath(segmentId, blockId, _blockFolder);
    flushFile(filepath);
}

void StorageModule::flushDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId, bool isParity) {

    string filepath = generateDeltaBlockPath(segmentId, blockId, deltaId, _blockFolder);
    flushFile(filepath);
}

//
// PRIVATE METHODS
//

/*
 void StorageModule::writeSegmentInfo(uint64_t segmentId, uint32_t segmentSize,
 string filepath) {

 // TODO: Database to be implemented

 }
 */

struct SegmentInfo StorageModule::readSegmentInfo(uint64_t segmentId) {
// TODO: Database to be implemented
    struct SegmentInfo segmentInfo;

    segmentInfo.segmentId = segmentId;
    segmentInfo.segmentPath = generateSegmentPath(segmentId, _segmentFolder);
    segmentInfo.segLength = getFilesize(segmentInfo.segmentPath);

    return segmentInfo;
}

uint32_t StorageModule::readFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, int priority) {

    bool isFinished = false;

#ifdef USE_IO_THREADS
    schedule(_iotp,
            prio_task_func(priority,
                    boost::bind(&StorageModule::doReadFile, this, filepath, buf,
                            offset, length, isCache, boost::ref(isFinished))));

    while (!isFinished) {
        usleep(IO_POLL_INTERVAL);
    }

    return length;
#else
    return doReadFile(filepath, buf, offset, length, isCache, isFinished);
#endif
}

uint32_t StorageModule::doReadFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, bool &isFinished) {

// cache and segment share different mutex
    lock_guard<mutex> lk(fileMutex[isCache]);

    debug("Read File :%s\n", filepath.c_str());

    FILE* file = openFile(filepath);

    if (file == NULL) { // cannot open file
        debug("%s\n", "Cannot read");
        perror("open");
        exit(-1);
    }

// Read file contents into buffer
    uint32_t byteRead = pread(fileno(file), buf, length, offset);

    if (byteRead != length) {
        debug_error(
                "ERROR: Length = %" PRIu32 ", Offset = %" PRIu64 ", byteRead = %" PRIu32 "\n",
                length, offset, byteRead);
        perror("pread()");
        exit(-1);
    }

    isFinished = true;

    return byteRead;
}

uint32_t StorageModule::writeFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, int priority) {

    bool isFinished = false;

#ifdef USE_IO_THREADS
    _iotp.schedule(
            prio_task_func(priority,
                    boost::bind(&StorageModule::doWriteFile, this, filepath,
                            buf, offset, length, isCache,
                            boost::ref(isFinished))));

    while (!isFinished) {
        usleep(IO_POLL_INTERVAL);
    }

    debug("Length = %" PRIu32 "\n", length);
    return length;
#else
    return doWriteFile(filepath, buf, offset, length, isCache, isFinished);
#endif
}

uint32_t StorageModule::doWriteFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, bool &isFinished) {

// cache and segment share different mutex
    lock_guard<mutex> lk(fileMutex[isCache]);

    FILE* file = openFile(filepath);

    if (file == NULL) { // cannot open file
        debug("%s\n", "Cannot write");
        perror("open");
        exit(-1);
    }

// Write file contents from buffer
    uint32_t byteWritten = pwrite(fileno(file), buf, length, offset);

    if (byteWritten != length) {
        debug_error("ERROR: Length = %d, byteWritten = %d\n", length,
                byteWritten);
        exit(-1);
    }

    isFinished = true;

    return byteWritten;
}

string StorageModule::generateSegmentPath(uint64_t segmentId,
        string segmentFolder) {

// append a '/' if not present
    if (segmentFolder[segmentFolder.length() - 1] != '/') {
        segmentFolder.append("/");
    }

    return segmentFolder + to_string(segmentId);
}

string StorageModule::generateBlockPath(uint64_t segmentId, uint32_t blockId,
        string blockFolder) {

// append a '/' if not present
    if (blockFolder[blockFolder.length() - 1] != '/') {
        blockFolder.append("/");
    }

    return blockFolder + to_string(segmentId) + "." + to_string(blockId);
}

string StorageModule::generateDeltaBlockPath(uint64_t segmentId,
        uint32_t blockId, uint32_t deltaId, string blockFolder) {

// append a '/' if not present
    if (blockFolder[blockFolder.length() - 1] != '/') {
        blockFolder.append("/");
    }

    return blockFolder + to_string(segmentId) + "." + to_string(blockId) + "."
            + to_string(deltaId);
}

/**
 * Create and open a new file
 */

FILE* StorageModule::createFile(string filepath) {

// open file for read/write
// create new if not exist
    FILE* filePtr;
    try {
        filePtr = _openedFile->get(filepath);
    } catch (out_of_range& oor) { // file pointer not found in cache
        filePtr = fopen(filepath.c_str(), "wb+");
        debug("OPEN1: %s\n", filepath.c_str());

        if (filePtr == NULL) {
            debug("%s\n", "Unable to create file!");
            return NULL;
        }

        // set buffer to zero to avoid memory leak
        //setvbuf(filePtr, NULL, _IONBF, 0);

        // add file pointer to map
        _openedFile->insert(filepath, filePtr);

        /*
         openedFileMutex.lock();
         _openedFile[filepath] = filePtr;
         openedFileMutex.unlock();
         */
    }

    return filePtr;
}

/**
 * Open an existing file, return pointer directly if file is already open
 */

FILE* StorageModule::openFile(string filepath) {

    FILE* filePtr = NULL;
    try {
        filePtr = _openedFile->get(filepath);
    } catch (out_of_range& oor) { // file pointer not found in cache
        filePtr = fopen(filepath.c_str(), "rb+");
        debug("OPEN2: %s\n", filepath.c_str());

        if (filePtr == NULL) {
            debug("Unable to open file at %s\n", filepath.c_str());
            perror("fopen()");
            return NULL;
        }

        // set buffer to zero to avoid memory leak
        // setvbuf(filePtr, NULL, _IONBF, 0);

        // add file pointer to map
        _openedFile->insert(filepath, filePtr);
    }

    return filePtr;
}

void StorageModule::tryCloseFile(string filepath) {
    FILE* filePtr = NULL;
    try {
        filePtr = _openedFile->get(filepath);
        fclose(filePtr);
        debug("CLOSE1: %s\n", filepath.c_str());
        _openedFile->remove(filepath);
    } catch (out_of_range& oor) { // file pointer not found in cache
        return;
    }
}

struct SegmentData StorageModule::getSegmentData(uint64_t segmentId,
        DataMsgType dataMsgType, string updateKey) {
    if (dataMsgType == UPLOAD) {
        {
            lock_guard<mutex> lk(uploadCacheMutex);
            if (!_segmentUploadCache.count(segmentId)) {
                debug_error("Segment cache not found %" PRIu64 "\n", segmentId);
                exit(-1);
            }
            return _segmentUploadCache[segmentId];
        }
    } else if (dataMsgType == UPDATE) {
        {
            lock_guard<mutex> lk(updateCacheMutex);
            if (!_segmentUpdateCache.count(updateKey)) {
                debug_error("Segment cache not found %" PRIu64 "\n", segmentId);
                exit(-1);
            }
            return _segmentUpdateCache[updateKey];
        }
    } else {
        debug_error("Invalid dataMsgType = %d\n", dataMsgType);
        exit(-1);
    }
    return {};
}

void StorageModule::updateBlockFreespace(uint32_t size) {
    if (isEnoughBlockSpace(size)) {
        _currentBlockUsage += size;
        _freeBlockSpace -= size;
    } else {
        debug_error("Block space not sufficient to save %s\n",
                formatSize(size).c_str());
        exit(-1);
    }
}

int32_t StorageModule::spareSegmentSpace(uint32_t newSegmentSize) {

    if (_maxSegmentCache < newSegmentSize) {
        return -1; // error: segment size larger than cache
    }

    while (_freeSegmentSpace < newSegmentSize) {
        uint64_t segmentId = 0;
        struct SegmentDiskCache segmentCache;

        segmentId = _segmentDiskCacheQueue.front();
        segmentCache = _segmentDiskCacheMap.get(segmentId);

        tryCloseFile(segmentCache.filepath);
        remove(segmentCache.filepath.c_str());

        // update size
        _freeSegmentSpace += segmentCache.length;
        _currentSegmentUsage -= segmentCache.length;

        // remove from queue and map
        _segmentDiskCacheQueue.remove(segmentId);
        _segmentDiskCacheMap.erase(segmentId);
    }

    return 0;

}

void StorageModule::putSegmentToDiskCache(uint64_t segmentId,
        SegmentData segmentCache) {

    lock_guard<mutex> lk(diskCacheMutex);

    debug("Before saving segment ID = %" PRIu64 ", cache = %s\n", segmentId,
            formatSize(_freeSegmentSpace).c_str());

    uint32_t segmentSize = segmentCache.info.segLength;

    if (!isEnoughSegmentSpace(segmentSize)) {
        // clear cache if space is not available
        if (spareSegmentSpace(segmentSize) == -1) {
            debug(
                    "Not enough space to cache segment! Segment Size = %" PRIu32 "\n",
                    segmentSize);
            return;
        }
    }

    debug("Spare space for saving segment ID = %" PRIu64 ", cache = %s\n",
            segmentId, formatSize(_freeSegmentSpace).c_str());

// write cache to disk
    const string filepath = generateSegmentPath(segmentId, _segmentFolder);
    createFile(filepath);

#ifdef USE_SEGMENT_CACHE
//TODO: segment cache length?
    uint64_t byteWritten = writeSegmentDiskCache(segmentId, segmentCache.buf, 0,
            segmentCache.segLength);
#else
    uint64_t byteWritten = segmentCache.bufLength;
#endif

    if (byteWritten != segmentCache.bufLength) {
        perror("Cannot saveSegmentToDisk");
        exit(-1);
    }
    flushSegmentDiskCache(segmentId);

    _currentSegmentUsage += segmentSize;
    _freeSegmentSpace -= segmentSize;

// save cache to map
    struct SegmentDiskCache segmentDiskCache;
    segmentDiskCache.filepath = generateSegmentPath(segmentId, _segmentFolder);
    segmentDiskCache.length = segmentCache.bufLength;
    segmentDiskCache.lastAccessedTime = {time(NULL), 0}; // set to current time

    _segmentDiskCacheMap.set(segmentId, segmentDiskCache);
    _segmentDiskCacheQueue.push_back(segmentId);

    debug("After saving segment ID = %" PRIu64 ", cache = %s\n", segmentId,
            formatSize(_freeSegmentSpace).c_str());

}

struct SegmentData StorageModule::getSegmentFromDiskCache(uint64_t segmentId) {
    struct SegmentData segmentData;
    struct SegmentDiskCache segmentDiskCache;
    {
        lock_guard<mutex> lk(diskCacheMutex);
        segmentDiskCache = _segmentDiskCacheMap.get(segmentId);
    }
    segmentData = readSegment(segmentId, 0, segmentDiskCache.length);
    return segmentData;
}

void StorageModule::clearSegmentDiskCache() {

    lock_guard<mutex> lk(diskCacheMutex);

    for (auto segment : _segmentDiskCacheQueue) {
        string filepath = _segmentFolder + to_string(segment);
        tryCloseFile(filepath);
        remove(filepath.c_str());
    }

    _segmentDiskCacheQueue.clear();
    _segmentDiskCacheMap.clear();

    _freeSegmentSpace = _maxSegmentCache;
    _currentSegmentUsage = 0;
}

list<uint64_t> StorageModule::getSegmentCacheQueue() {
    return _segmentDiskCacheQueue;
}

uint32_t StorageModule::getCurrentBlockCapacity() {
    return _currentBlockUsage;
}

uint32_t StorageModule::getCurrentSegmentCache() {
    return _currentSegmentUsage;
}

uint32_t StorageModule::getFreeBlockSpace() {
    return _freeBlockSpace;
}

uint32_t StorageModule::getFreeSegmentSpace() {
    return _freeSegmentSpace;
}

void StorageModule::setMaxBlockCapacity(uint32_t max_block) {
    _maxBlockCapacity = max_block;
}

void StorageModule::setMaxSegmentCache(uint32_t max_segment) {
    _maxSegmentCache = max_segment;
}

uint32_t StorageModule::getMaxBlockCapacity() {
    return _maxBlockCapacity;
}

uint32_t StorageModule::getMaxSegmentCache() {
    return _maxSegmentCache;
}

bool StorageModule::isEnoughBlockSpace(uint32_t size) {
    return size <= _freeBlockSpace ? true : false;
}

bool StorageModule::isEnoughSegmentSpace(uint32_t size) {
    return size <= _freeSegmentSpace ? true : false;
}

uint32_t StorageModule::getCombinedLength(vector<offset_length_t> offsetLength) {
    uint32_t combinedLength = 0;
    for (auto offsetLengthPair : offsetLength) {
        combinedLength += offsetLengthPair.second;
    }
    return combinedLength;
}


RWMutex* StorageModule::obtainRWMutex(string blockKey) {
    // obtain rwmutex for this segment
    _deltaRWMutexMapMutex.lock();
    RWMutex* rwmutex;
    if (_deltaRWMutexMap.count(blockKey) == 0) {
        rwmutex = new RWMutex();
        _deltaRWMutexMap[blockKey] = rwmutex;
    } else {
        rwmutex = _deltaRWMutexMap[blockKey];
    }
    _deltaRWMutexMapMutex.unlock();
    return rwmutex;
}

/*
DeltaLocation StorageModule::getDeltaLocation (uint64_t segmentId, uint32_t blockId, uint32_t deltaId) {
    const string blockKey = to_string(segmentId) + "." + to_string(blockId);

    RWMutex* rwmutex = obtainRWMutex(blockKey);
    readLock rdlock(*rwmutex);

    vector<DeltaLocation> deltaLocationList = _deltaLocationMap[blockKey];

    for (DeltaLocation deltaLocation: deltaLocationList) {
        debug ("DeltaLocation deltaId = %" PRIu32 " isReserve = %d\n", deltaLocation.deltaId, deltaLocation.isReserveSpace);
        if (deltaLocation.deltaId == deltaId) {
            return deltaLocation;
        }
    }

    debug_error ("DeltaID %" PRIu32 " not found for %s\n", deltaId, blockKey.c_str());
    exit (-1);
}
*/

string StorageModule::getBlockKey (uint64_t segmentId, uint32_t blockId) {
    return to_string(segmentId) + "." + to_string(blockId);
}

string StorageModule::getBlockKey (string segmentId, string blockId) {
    return segmentId + "." + blockId;
}

string StorageModule::generateDeltaKey (uint64_t segmentId, uint32_t blockId, uint32_t deltaId) {
    return to_string(segmentId) + "." + to_string(blockId) + "." + to_string(deltaId);
}
