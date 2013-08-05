#include "filedatacache.hh"

#include <openssl/md5.h>

#include "client.hh"
#include "client_communicator.hh"

#include "../common/debug.hh"
#include "../common/memorypool.hh"
#include "../common/convertor.hh"	//md5ToHex()
#include "../coding/allcoding.hh"

extern Client* client;
extern uint32_t _clientId;
extern ClientCommunicator* _clientCommunicator;
extern ConfigLayer* _configLayer;

FileDataCache::FileDataCache() {
    // TODO: Read from XML
    _segmentSize = 10 * 1024 * 1024;
    _codingScheme = RAID0_CODING;
    _codingSetting = Raid0Coding::generateSetting(2);
    _lruSizeLimit = 10;
    _writeBufferSize = 1;
    _writeBuffer = new RingBuffer<uint64_t>(_writeBufferSize);
    _numWriteThread = 10;
    for (uint32_t i = 0; i < _numWriteThread; ++i) {
        _writeThreads.push_back(thread(&FileDataCache::writeBackThread, this));
    }
    _prefetchBufferSize = 10;
    _prefetchBuffer = new RingBuffer<std::pair<uint64_t, uint32_t> >(
            _prefetchBufferSize);
    _numPrefetchThread = 0;
    for (uint32_t i = 0; i < _numPrefetchThread; ++i) {
        _prefetchThreads.push_back(thread(&FileDataCache::doPrefetch, this));
    }

    _storageModule = client->getStorageModule();
}

uint32_t FileDataCache::readDataCache(uint64_t segmentId, uint32_t primary,
        void* buf, uint32_t size, uint32_t offset) {
    debug("Read Cache %" PRIu64 " at %" PRIu32 " for %" PRIu32 "\n", segmentId,
            offset, size);
    if (_segmentStatus.count(segmentId) == 1) {
        debug ("Cached %" PRIu64 "\n", segmentId);
        SegmentData segmentCache = _storageModule->getSegmentCache(segmentId);
        memcpy(buf, segmentCache.buf + offset, size);
    } else {
        debug ("Not Cached %" PRIu64 "\n", segmentId);

        uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
        struct SegmentData segmentCache = client->getSegment(
                _clientId, sockfd, segmentId);
        memcpy(buf, segmentCache.buf + offset, size);

        _segmentStatus[segmentId] = CLEAN;
        _segmentPrimary[segmentId] = primary;
    }
    updateLru(segmentId);
    return size;
}

uint32_t FileDataCache::writeDataCache(uint64_t segmentId, uint32_t primary,
        const void* buf, uint32_t size, uint32_t offset, FileType fileType) {
    // TODO: Check - Forbid Write to Sealed Segment
    struct SegmentData segmentCache;
    if (_segmentStatus.count(segmentId) == 0) {
        _segmentPrimary[segmentId] = primary;
        segmentCache.info.segmentId = segmentId;
        segmentCache.info.segLength = _segmentSize;
        segmentCache.buf = (char*) MemoryPool::getInstance().poolMalloc(
                _segmentSize);
    } else {
        segmentCache = _storageModule->getSegmentCache(segmentId);
    }

    segmentCache.fileType = fileType;

    // Bound Check
    if (offset + size > _segmentSize) {
        perror("exceed boundary");
        return -1;
    }

    // Copy new update/data into segmentData struct
    memcpy(segmentCache.buf + offset, buf, size);

    // Inline-update: Add offset, length pair
    segmentCache.info.offlenVector.push_back(make_pair(offset, size));

    if (offset + size > segmentCache.info.segLength)
        segmentCache.info.segLength = offset + size;

    _storageModule->setSegmentCache(segmentId, segmentCache);
    _segmentStatus[segmentId] = DIRTY;

    updateLru(segmentId);
    return size;
}

void FileDataCache::closeDataCache(uint64_t segmentId, bool sync) {
    if (_segmentStatus.count(segmentId) == 0) {
        debug("Segment %" PRIu64 " not Cached\n", segmentId);
        return;
    }

    /*
     _lruListMutex.lock();
     list<uint64_t>::iterator tempIt;
     tempIt = _segment2LruMap[segmentId];
     _segmentLruList.erase(tempIt);
     _segment2LruMap.erase(segmentId);
     _lruListMutex.unlock();
     */

    debug("Closing Data Cache %" PRIu64 "\n", segmentId);

    // Read Cache
    if (_segmentStatus[segmentId] == CLEAN) {
        _storageModule->closeSegment(segmentId);
        _segmentStatus.erase(segmentId);
        _segmentPrimary.erase(segmentId);
        _prefetchBitmap.erase(segmentId);
        return;
    }

    //_segmentStatus.erase(segmentId);
    // Write Cache
    if (sync)
        doWriteBack(segmentId);
    else
        writeBack(segmentId);

    // TODO: Convert to Read Cache
    //_segmentStatus[segmentId] = CLEAN;
}

void FileDataCache::prefetchSegment(uint64_t segmentId, uint32_t primary) {
    if (_prefetchBitmap.count(segmentId) == 0) {
        if (_segmentStatus.count(segmentId) == 0) {
            _prefetchBitmap[segmentId] = true;
            _prefetchBuffer->push(make_pair(segmentId, primary));
        }
    }
}

void FileDataCache::writeBack(uint64_t segmentId) {
    _writeBuffer->push(segmentId);
}

void FileDataCache::doPrefetch() {
    std::pair<uint64_t, uint32_t> tempPair;
    while (1) {
        tempPair = _prefetchBuffer->pop();
        uint64_t segmentId = tempPair.first;
        uint32_t primary = tempPair.second;
        uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);

        debug("Prefetch %" PRIu64 "\n", segmentId);

        if (_segmentStatus.count(segmentId) == 0) {
            _segmentStatus[segmentId] = CLEAN;
            _segmentPrimary[segmentId] = primary;
            debug("Create Cache for %" PRIu64 "\n", segmentId);
        }

        client->getSegment(_clientId, sockfd, segmentId);
        updateLru(segmentId);
    }
}

void FileDataCache::writeBackThread() {
    while (1) {
        uint64_t segmentId = _writeBuffer->pop();
        doWriteBack(segmentId);
    }
    return;
}

void FileDataCache::doWriteBack(uint64_t segmentId) {

    if (_segmentStatus.count(segmentId) == 0) {
        return;
    }

    SegmentStatus segmentStatus = _segmentStatus[segmentId];
    uint32_t primary = _segmentPrimary[segmentId];
    struct SegmentData segmentData = _storageModule->getSegmentCache(segmentId);
    _segmentPrimary.erase(segmentId);
    _segmentStatus.erase(segmentId);

    if ((segmentData.info.segLength == 0) || (segmentStatus != DIRTY)) {
        MemoryPool::getInstance().poolFree(segmentData.buf);
        _segmentStatus.erase(segmentId);
        return;
    }

    uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);

    unsigned char checksum[MD5_DIGEST_LENGTH];
    memset(checksum, 0, MD5_DIGEST_LENGTH);

#ifdef USE_CHECKSUM
    MD5((unsigned char*) segmentData.buf, segmentData.info.segLength, checksum);
#endif

    debug("Write Back Segment %" PRIu64 " ,Size %" PRIu32 "\n", segmentId,
            segmentData.info.segLength);
    _clientCommunicator->sendSegment(_clientId, sockfd, segmentData,
            _codingScheme, _codingSetting, md5ToHex(checksum));
    _storageModule->closeSegment(segmentId);
    _segmentStatus.erase(segmentId);
    return;
}

void FileDataCache::updateLru(uint64_t segmentId) {
    list<uint64_t>::iterator tempIt;
    if (_segment2LruMap.count(segmentId) == 1) {
        tempIt = _segment2LruMap[segmentId];
        // move tempIt to _segmentLruList.end() 
        _segmentLruList.splice(_segmentLruList.end(), _segmentLruList, tempIt);
    }
    // Create LRU Record
    else {
        if (_segmentLruList.size() >= _lruSizeLimit) {
            uint64_t segmentToClose = _segmentLruList.front();
            _segmentLruList.pop_front();
            _segment2LruMap.erase(segmentToClose);
            closeDataCache(segmentToClose);
        }
        tempIt = _segmentLruList.insert(_segmentLruList.end(), segmentId);
        _segment2LruMap[segmentId] = tempIt;
    }
    return;

}
