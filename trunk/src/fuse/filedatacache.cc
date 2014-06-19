#include "filedatacache.hh"
#include "client.hh"
#include "client_communicator.hh"
#include "../common/debug.hh"
#include "../common/memorypool.hh"
#include "../coding/allcoding.hh"
#include "../common/convertor.hh"

extern Client* client;
extern uint32_t _clientId;
extern ClientCommunicator* _clientCommunicator;
extern ConfigLayer* configLayer;

FileDataCache::FileDataCache() {
    // TODO: Read from XML

    _segmentSize = stringToByte(configLayer->getConfigString("Fuse>segmentSize"));
    int coding = configLayer->getConfigInt("Fuse>codingScheme");
    int n, k, m, w;
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
            k = configLayer->getConfigInt("Fuse>RS>K");
            m = configLayer->getConfigInt("Fuse>RS>M");
            w = configLayer->getConfigInt("Fuse>RS>W");
            _codingScheme = RS_CODING;
            _codingSetting = RSCoding::generateSetting(k, m, w);
            break;
        case 4:
            n = configLayer->getConfigInt("Fuse>EMBR>N");
            k = configLayer->getConfigInt("Fuse>EMBR>K");
            w = configLayer->getConfigInt("Fuse>EMBR>W");
            _codingScheme = EMBR_CODING;
            _codingSetting = EMBRCoding::generateSetting(n, k, w);
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
        case 7:
            k = configLayer->getConfigInt("Fuse>CAUCHY>C_K");
            m = configLayer->getConfigInt("Fuse>CAUCHY>C_M");
            w = configLayer->getConfigInt("Fuse>CAUCHY>C_W");
            _codingScheme = CAUCHY;
            _codingSetting = CauchyCoding::generateSetting(k, m, w);
            break;
        default:
            debug("Invalid Test = %d\n", coding);
            break;
    }

    _lruSizeLimit = configLayer->getConfigInt("Fuse>lruSize");
    _writeBufferSize = configLayer->getConfigInt("Fuse>writeBufferSize");
    _writeBuffer = new RingBuffer<uint64_t>(_writeBufferSize);
    _numWriteThread = configLayer->getConfigInt("Fuse>numWriteThread");
    for (uint32_t i = 0; i < _numWriteThread; ++i) {
        _writeThreads.push_back(thread(&FileDataCache::writeBackThread, this));
    }

    _prefetchBufferSize = configLayer->getConfigInt("Fuse>prefetchBufferSize");
    _prefetchBuffer = new RingBuffer<std::pair<uint64_t, uint32_t> >(
            _prefetchBufferSize);
    _numPrefetchThread = configLayer->getConfigInt("Fuse>numPrefetchThread");
    for (uint32_t i = 0; i < _numPrefetchThread; ++i) {
        _prefetchThreads.push_back(thread(&FileDataCache::prefetchThread, this));
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

    if (_writeBackSegmentPrimary.count(segmentId) != 0) {
        rdlock.unlock();
        debug("Read need flush %" PRIu64 " at %" PRIu32 " for %" PRIu32 "\n", segmentId,
                offset, size);
        writeLock wtlock(*rwmutex);
        if (_writeBackSegmentPrimary.count(segmentId) != 0) {
            doWriteBack(segmentId);
        }
        wtlock.unlock();
        rdlock.lock();
        debug("Read flushed %" PRIu64 " at %" PRIu32 " for %" PRIu32 "\n", segmentId,
                offset, size);
    }

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
        if (_storageModule->locateSegmentCache(segmentId) == 0) {
            segmentCache.info.segmentId = segmentId;
            segmentCache.info.segLength = _segmentSize;
            segmentCache.buf = (char*) MemoryPool::getInstance().poolMalloc(
                _segmentSize);
        } else {
            debug ("segment id %" PRIu64 " getSegmentcache \n", segmentId);
            segmentCache = _storageModule->getSegmentCache(segmentId);
        }
    } else {
        debug ("segment id %" PRIu64 " getSegmentcache \n", segmentId);
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
        //debug("Closing Data Cache %" PRIu64 "\n", segmentId);

        // If the cache is not modified, just free and return
        if (_writeBackSegmentPrimary.count(segmentId) == 0) {
            _storageModule->closeSegment(segmentId);
            _writeBackSegmentPrimary.erase(segmentId);
            rwmutex->unlock();
            return;
        }

        // IF the cache has to be write back, i.e. Write Cache
        if (sync) {
            doWriteBack(segmentId);
        } else {
            writeBack(segmentId);
        }

        rwmutex->unlock();
    } else {
        debug("TRY LOCK SEGMENT %" PRIu64 " FAILED\n", segmentId);
    }
}

void FileDataCache::prefetchSegment(uint64_t segmentId, uint32_t primary) {
    lock_guard<mutex> lk(_prefetchBitmapMutex);
    if (_prefetchBitmap.count(segmentId) == 0) {
        _prefetchBitmap[segmentId] = true;
        _prefetchBuffer->push(make_pair(segmentId, primary));
    }
}

void FileDataCache::writeBack(uint64_t segmentId) {
    _writeBuffer->push(segmentId);
}

void FileDataCache::prefetchThread() {
    std::pair<uint64_t, uint32_t> tempPair;
    while (1) {
        tempPair = _prefetchBuffer->pop();

        uint64_t segmentId = tempPair.first;
        uint32_t primary = tempPair.second;

        RWMutex* rwmutex = obtainRWMutex(segmentId);
        readLock rdlock(*rwmutex);
        debug("Prefetch %" PRIu64 "\n", segmentId);

        if (_writeBackSegmentPrimary.count(segmentId) != 0) {
            rdlock.unlock();
            debug("Prefetch need flush %" PRIu64 "\n", segmentId);
            writeLock wtlock(*rwmutex);
            if (_writeBackSegmentPrimary.count(segmentId) != 0) {
                doWriteBack(segmentId);
            }
            wtlock.unlock();
            rdlock.lock();
            debug("Prefetch flushed %" PRIu64 "\n", segmentId);
        }

        uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
        client->getSegment(_clientId, sockfd, segmentId);
        updateLru(segmentId);
    }
}

void FileDataCache::writeBackThread() {
    while (1) {
        uint64_t segmentId = _writeBuffer->pop();
        // obtain unique lock 
        RWMutex* rwmutex = obtainRWMutex(segmentId);
        writeLock wtlock(*rwmutex);
        if (_writeBackSegmentPrimary.count(segmentId) != 0) {
            doWriteBack(segmentId);
        }
    }
    return;
}

void FileDataCache::doWriteBack(uint64_t segmentId) {

    if (!_storageModule->locateSegmentCache(segmentId)) {
        return;
    }

    uint32_t primary = _writeBackSegmentPrimary.get(segmentId);
    debug ("segment id %" PRIu64 " getSegmentcache \n", segmentId);
    struct SegmentData segmentData = _storageModule->getSegmentCache(segmentId);
    _writeBackSegmentPrimary.erase(segmentId);

    if (segmentData.info.segLength == 0) {
        debug ("Empty Segment ID = %" PRIu64 " SegLength = %" PRIu32 "\n", segmentData.info.segmentId, segmentData.info.segLength);
        _storageModule->closeSegment(segmentId);
        return;
    }

    uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);

    debug("Write Back Segment %" PRIu64 " ,Size %" PRIu32 "\n", segmentId,
            segmentData.info.segLength);
    _clientCommunicator->sendSegment(_clientId, sockfd, segmentData,
            _codingScheme, _codingSetting);
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
