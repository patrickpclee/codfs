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
#include "../coding/coding.hh"

// global variable defined in each component
extern ConfigLayer* configLayer;

mutex fileMutex[2];
mutex uploadCacheMutex;
mutex updateCacheMutex;
mutex diskCacheMutex;

StorageModule::StorageModule() {
    _openedFile = new FileLruCache<string, FILE*>(MAX_OPEN_FILES);
    _segmentUploadCache = {};
    _segmentUpdateCache = {};
    _blockFolder = configLayer->getConfigString("Storage>BlockLocation");

    _updateScheme = configLayer->getConfigInt("Storage>UpdateScheme");
    if (_updateScheme == PLR) {
        _reservedSpaceSize = stringToByte(configLayer->getConfigString("Storage>ReservedSpaceSize"));
    } else {
        _reservedSpaceSize = 0; // important
    }

    struct stat st;
    if (stat(_blockFolder.c_str(), &st) != 0) {
        debug("%s does not exist, make directory automatically\n",
                _blockFolder.c_str());
        if (mkdir(_blockFolder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
            perror("mkdir");
            exit(-1);
        }
    }

    // Unit in StorageModule: Bytes
    _maxBlockCapacity = stringToByte(
            configLayer->getConfigString("Storage>BlockCapacity"));

    cout << "=== STORAGE ===" << endl;
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

    _freeBlockSpace = _maxBlockCapacity;
    _currentBlockUsage = 0;

    struct dirent* dent;
    DIR* srcdir;

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
        _freeBlockSpace -= (uint64_t)st.st_size;
        _currentBlockUsage += (uint64_t)st.st_size;

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

void StorageModule::createSegmentTransferCache(uint64_t segmentId, uint32_t segLength,
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

        _deltaIdMap.set(blockKey, 0);
        _deltaLocationMap.set(blockKey, vector<DeltaLocation>());
        ReserveSpaceInfo reserveSpaceInfo;
        reserveSpaceInfo.currentOffset = length;
        reserveSpaceInfo.remainingReserveSpace = 0;
        reserveSpaceInfo.blockSize = length;
        _reserveSpaceMap.set(blockKey, reserveSpaceInfo);
    }
}

void StorageModule::reserveBlockSpace(uint64_t segmentId, uint32_t blockId,
        uint32_t offset, uint32_t blockSize, uint32_t reserveLength) {

    const string blockKey = getBlockKey (segmentId, blockId);

    const string filepath = generateBlockPath(segmentId, blockId, _blockFolder);

    FILE* ptr = openFile(filepath);

    {
        RWMutex* rwmutex = obtainRWMutex(blockKey);
        writeLock wtlock(*rwmutex);

        if (posix_fallocate (fileno (ptr), offset, reserveLength) < 0) {
            debug_error ("Failed to reserve space: %s\n", filepath.c_str());
            exit (-1);
        }

        ReserveSpaceInfo reserveSpaceInfo;
        reserveSpaceInfo.currentOffset = blockSize;
        reserveSpaceInfo.remainingReserveSpace = _reservedSpaceSize;
        reserveSpaceInfo.blockSize = blockSize;
        _reserveSpaceMap.set(blockKey, reserveSpaceInfo);
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
    uint32_t curDeltaId = _deltaIdMap.increment(blockKey) - 1;
    return curDeltaId;
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
    vector<offset_length_t> offsetLength = _deltaOffsetLength.get(deltaKey);
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

BlockData StorageModule::getBlock (uint64_t segmentId, uint32_t blockId, bool isParity, vector<offset_length_t> symbols, bool needLock) {
    if (isParity) {
        if (_updateScheme == FO) {
            return readBlock(segmentId, blockId, symbols);
        } else {
            return getMergedBlock(segmentId, blockId, isParity, true);
        }
    } else {
        if (_updateScheme == FL) {
            return getMergedBlock(segmentId, blockId, isParity, true);
        } else {
            return readBlock(segmentId, blockId, symbols);
        }
    }
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
    BlockData delta;
    for (DeltaLocation deltaLocation : _deltaLocationMap.get(blockKey)) {
        const uint32_t deltaId = deltaLocation.deltaId;

        // read from reserved space if delta is inside
        if (deltaLocation.isReserveSpace) {
            debug ("Reading from Reserve Segment ID = %" PRIu64 " Block ID = %" PRIu32 " Delta ID = %" PRIu32 "\n", segmentId, blockId, deltaId);
            string deltaKey = generateDeltaKey (segmentId, blockId, deltaId);
            vector<offset_length_t> offsetLength = _deltaOffsetLength.get(deltaKey);
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
    vector<DeltaLocation> &deltaLocationList = _deltaLocationMap.get(blockKey);
    for (DeltaLocation deltaLocation : deltaLocationList) {
        if (!deltaLocation.isReserveSpace) {
            const string deltaBlockPath = generateDeltaBlockPath(segmentId, blockId, deltaLocation.deltaId,
                    _blockFolder);

            // remove delta from disk
            tryCloseFile(deltaBlockPath);
            remove(deltaBlockPath.c_str());
        }
    }

    // remove all delta information
    deltaLocationList.clear();
    _reserveSpaceMap.get(blockKey).remainingReserveSpace = _reservedSpaceSize;
    _reserveSpaceMap.get(blockKey).currentOffset = blockData.info.blockSize;
}

uint32_t StorageModule::writeSegmentTransferCache(uint64_t segmentId, char* buf,
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

    ReserveSpaceInfo &reserveSpaceInfo = _reserveSpaceMap.get(blockKey);

    DeltaLocation deltaLocation;
    deltaLocation.blockId = blockId;
    deltaLocation.deltaId = deltaId;

    uint32_t currentOffset = 0;
    string filepath = "";
    if (isParity && combinedLength <= _reservedSpaceSize) {
        if (reserveSpaceInfo.remainingReserveSpace < combinedLength) {
            // merge existing block and write again
            debug ("need merge remaining = %" PRIu32 " length = %" PRIu32 " deltaId = %" PRIu32 "\n", reserveSpaceInfo.remainingReserveSpace, combinedLength, deltaId);
            mergeBlock(segmentId, blockId, true);

#ifdef LATENCY_TEST
            typedef chrono::high_resolution_clock Clock;
            Clock::time_point t0 = Clock::now();
            cout << "MERGE TIME" << t0.time_since_epoch().count() << endl;
#endif
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

    _deltaOffsetLength.set(deltaKey, offsetLength);
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

void StorageModule::closeSegmentTransferCache(uint64_t segmentId,
        DataMsgType dataMsgType, string updateKey) {

    if (dataMsgType == UPLOAD) {
        // close cache
        struct SegmentData segmentCache = getSegmentTransferCache(segmentId,
                dataMsgType);
        MemoryPool::getInstance().poolFree(segmentCache.buf);

        {
            lock_guard<mutex> lk(uploadCacheMutex);
            _segmentUploadCache.erase(segmentId);
        }
    } else if (dataMsgType == UPDATE) {
        // close cache
        struct SegmentData segmentCache = getSegmentTransferCache(segmentId, dataMsgType,
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

void StorageModule::flushFile(string filePath) {
    FILE* filePtr = NULL;
    try {
        filePtr = _openedFile->get(filePath);
        fflush(filePtr);
#ifdef USE_FSYNC
        fsync(fileno(filePtr));
#endif
    } catch (out_of_range& oor) { // file pointer not found in cache
        return; // file already closed by cache, do nothing
    }
    return;
}


void StorageModule::flushBlock(uint64_t segmentId, uint32_t blockId) {

    string filepath = generateBlockPath(segmentId, blockId, _blockFolder);
    flushFile(filepath);
}

void StorageModule::flushDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId, bool isParity) {

    string filepath = generateDeltaBlockPath(segmentId, blockId, deltaId, _blockFolder);
    flushFile(filepath);
}

uint32_t StorageModule::readFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, int priority) {

// cache and segment share different mutex
    //lock_guard<mutex> lk(fileMutex[isCache]);

    debug("Read File :%s\n", filepath.c_str());

    FILE* file = openFile(filepath);

    if (file == NULL) { // cannot open file
        debug("%s\n", "Cannot read");
        perror("open");
        exit(-1);
    }

#ifndef NO_WRITE
// Read file contents into buffer
    uint32_t byteRead = pread(fileno(file), buf, length, offset);
#else
	uint32_t byteRead = length;
#endif

    if (byteRead != length) {
        debug_error(
                "ERROR: Length = %" PRIu32 ", Offset = %" PRIu64 ", byteRead = %" PRIu32 "\n",
                length, offset, byteRead);
        perror("pread()");
        exit(-1);
    }

    return byteRead;
}

uint32_t StorageModule::writeFile(string filepath, char* buf, uint64_t offset,
        uint32_t length, bool isCache, int priority) {

// cache and segment share different mutex
    //lock_guard<mutex> lk(fileMutex[isCache]);

    FILE* file = openFile(filepath);

    if (file == NULL) { // cannot open file
        debug("%s\n", "Cannot write");
        perror("open");
        exit(-1);
    }

#ifndef NO_WRITE
// Write file contents from buffer
    uint32_t byteWritten = pwrite(fileno(file), buf, length, offset);
#else
	uint32_t byteWritten = length;
#endif

    if (byteWritten != length) {
        debug_error("ERROR: Length = %d, byteWritten = %d\n", length,
                byteWritten);
        exit(-1);
    }

    return byteWritten;
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

struct SegmentData StorageModule::getSegmentTransferCache(uint64_t segmentId,
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
        _currentBlockUsage += (uint64_t)size;
        _freeBlockSpace -= (uint64_t)size;
    } else {
        debug_error("Block space not sufficient to save %s\n",
                formatSize(size).c_str());
        exit(-1);
    }
}

uint64_t StorageModule::getCurrentBlockCapacity() {
    return _currentBlockUsage;
}

uint64_t StorageModule::getFreeBlockSpace() {
    return _freeBlockSpace;
}

void StorageModule::setMaxBlockCapacity(uint32_t max_block) {
    _maxBlockCapacity = max_block;
}

uint64_t StorageModule::getMaxBlockCapacity() {
    return _maxBlockCapacity;
}

bool StorageModule::isEnoughBlockSpace(uint32_t size) {
    return (uint64_t)size <= _freeBlockSpace ? true : false;
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

string StorageModule::getBlockKey (uint64_t segmentId, uint32_t blockId) {
    return to_string(segmentId) + "." + to_string(blockId);
}

string StorageModule::getBlockKey (string segmentId, string blockId) {
    return segmentId + "." + blockId;
}

string StorageModule::generateDeltaKey (uint64_t segmentId, uint32_t blockId, uint32_t deltaId) {
    return to_string(segmentId) + "." + to_string(blockId) + "." + to_string(deltaId);
}
