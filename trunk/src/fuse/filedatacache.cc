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
extern ConfigLayer* configLayer;

FileDataCache::FileDataCache() {
    // TODO: Read from XML

    _segmentSize = stringToByte(configLayer->getConfigString("Fuse>segmentSize"));
    int coding = configLayer->getConfigInt("Fuse>codingScheme");
    int n, k, m;
    switch (coding) {
        case 0:
            n = configLayer->getConfigInt("Fuse>RAID0>N");
            _codingScheme = RAID0_CODING;
            _codingSetting = Raid0Coding::generateSetting(n);
            break;
        case 1:
            n = configLayer->getConfigInt("Fuse>RAID1>N");
            _codingScheme = RAID1_CODING;
            _codingSetting = Raid1Coding::generateSetting(n);
            break;
        case 2:
            n = configLayer->getConfigInt("Fuse>RAID5>N");
            _codingScheme = RAID5_CODING;
            _codingSetting = Raid5Coding::generateSetting(n);
            break;
        case 3:
            n = configLayer->getConfigInt("Fuse>RS>N");
            k = configLayer->getConfigInt("Fuse>RS>K");
            m = configLayer->getConfigInt("Fuse>RS>M");
            _codingScheme = RS_CODING;
            _codingSetting = RSCoding::generateSetting(n, k, m);
            break;
        case 4:
            n = configLayer->getConfigInt("Fuse>EMBR>N");
            k = configLayer->getConfigInt("Fuse>EMBR>K");
            m = configLayer->getConfigInt("Fuse>EMBR>M");
            _codingScheme = EMBR_CODING;
            _codingSetting = EMBRCoding::generateSetting(n, k, m);
            break;
        case 5:
            n = configLayer->getConfigInt("Fuse>RDP>N");
            _codingScheme = RDP_CODING;
            _codingSetting = RDPCoding::generateSetting(n);
            break;
        case 6:
            n = configLayer->getConfigInt("Fuse>EVENODD>N");
            _codingScheme = EVENODD_CODING;
            _codingSetting = EvenOddCoding::generateSetting(n);
            break;
        default:
            debug("Invalid Test = %d\n", coding);
            break;
    }

    _lruSizeLimit = 10;
    _writeBufferSize = configLayer->getConfigInt("Fuse>writeBufferSize");
    _writeBuffer = new RingBuffer<uint64_t>(_writeBufferSize);
    _numWriteThread = configLayer->getConfigInt("Fuse>numWriteThread");
    for (uint32_t i = 0; i < _numWriteThread; ++i) {
        _writeThreads.push_back(thread(&FileDataCache::writeBackThread, this));
    }

    // desperated
    _prefetchBufferSize = 10;
    _prefetchBuffer = new RingBuffer<std::pair<uint64_t, uint32_t> >(
            _prefetchBufferSize);
    _numPrefetchThread = 0;
    for (uint32_t i = 0; i < _numPrefetchThread; ++i) {
        _prefetchThreads.push_back(thread(&FileDataCache::doPrefetch, this));
    }

    _storageModule = client->getStorageModule();
}

RWMutex* FileDataCache::obtainRWMutex(uint64_t segmentId) {
    // obtain rwmutex for this segment
    _segmentRWMutexMapMutex.lock();
    RWMutex* rwmutex;
    if (_segmentRWMutexMap.count(segmentId) == 0) {
        rwmutex = new RWMutex();
        _segmentRWMutexMap[segmentId] = rwmutex; 
    } else {
        rwmutex = _segmentRWMutexMap[segmentId];
    }
    _segmentRWMutexMapMutex.unlock();
    return rwmutex;
}

uint32_t FileDataCache::readDataCache(uint64_t segmentId, uint32_t primary,
        void* buf, uint32_t size, uint32_t offset) {
    // shared lock read
    RWMutex* rwmutex = obtainRWMutex(segmentId);
    readLock rdlock(*rwmutex);

    debug("Read %" PRIu64 " at %" PRIu32 " for %" PRIu32 "\n", segmentId,
            offset, size);

    // no matter whether cached, getSegment check cache first
    uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
    struct SegmentData segmentCache = client->getSegment(
            _clientId, sockfd, segmentId);
    memcpy(buf, segmentCache.buf + offset, size);
    updateLru(segmentId);
    return size;
}

uint32_t FileDataCache::writeDataCache(uint64_t segmentId, uint32_t primary,
        const void* buf, uint32_t size, uint32_t offset, FileType fileType) {
    // TODO: Check - Forbid Write to Sealed Segment

    // obtain unique lock 
    RWMutex* rwmutex = obtainRWMutex(segmentId);
    writeLock wtlock(*rwmutex);

    struct SegmentData segmentCache;
    if (_writeBackSegmentPrimary.count(segmentId) == 0) {
        _writeBackSegmentPrimary.set(segmentId, primary);
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

    updateLru(segmentId);
    return size;
}

void FileDataCache::closeDataCache(uint64_t segmentId, bool sync) {

    // obtain unique lock 
    RWMutex* rwmutex = obtainRWMutex(segmentId);
    //debug("TRY LOCK SEGMENT %" PRIu64 "\n", segmentId);
    if (rwmutex->try_lock()) 
    {
        debug("Closing Data Cache %" PRIu64 "\n", segmentId);

        // If the cache is not modified, just free and return
        if (_writeBackSegmentPrimary.count(segmentId) == 0) {
            _storageModule->closeSegment(segmentId);
            _writeBackSegmentPrimary.erase(segmentId);
            rwmutex->unlock();
            return;
        }

        // IF the cache has to be write back, i.e. Write Cache
        if (sync)
            doWriteBack(segmentId);
        else
            writeBack(segmentId);

        rwmutex->unlock();
    } else {
        debug("TRY LOCK SEGMENT %" PRIu64 " FAILED\n", segmentId);
    }
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
            _writeBackSegmentPrimary.set(segmentId, primary);
            debug("Create Cache for %" PRIu64 "\n", segmentId);
        }

        client->getSegment(_clientId, sockfd, segmentId);
        //updateLru(segmentId);
    }
}

void FileDataCache::writeBackThread() {
    while (1) {
        uint64_t segmentId = _writeBuffer->pop();
        // obtain unique lock 
        RWMutex* rwmutex = obtainRWMutex(segmentId);
        writeLock wtlock(*rwmutex);
        doWriteBack(segmentId);
    }
    return;
}

void FileDataCache::doWriteBack(uint64_t segmentId) {

    uint32_t primary = _writeBackSegmentPrimary.get(segmentId);
    struct SegmentData segmentData = _storageModule->getSegmentCache(segmentId);
    _writeBackSegmentPrimary.erase(segmentId);

    if (segmentData.info.segLength == 0) {
        debug ("Empty Segment ID = %" PRIu64 " SegLength = %" PRIu32 "\n", segmentData.info.segmentId, segmentData.info.segLength);
        _storageModule->closeSegment(segmentId);
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
    return;
}

void FileDataCache::updateLru(uint64_t segmentId) {
    lock_guard<std::mutex> lk(_lruMutex);

    list<uint64_t>::iterator tempIt;
    if (_segment2LruMap.count(segmentId) == 1) {
        tempIt = _segment2LruMap[segmentId];
        // move tempIt to _segmentLruList.end() 
        _segmentLruList.splice(_segmentLruList.end(), _segmentLruList, tempIt);
    }
    else {
        // Create LRU Record
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
