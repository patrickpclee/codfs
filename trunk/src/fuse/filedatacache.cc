#include "filedatacache.hh"

#include <openssl/md5.h>

#include "client.hh"
#include "client_communicator.hh"
#include "client_storagemodule.hh"

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
	_codingSetting = Raid0Coding::generateSetting(1);
	_lruSizeLimit = 10;
	_writeBufferSize = 1;
	_writeBuffer = new RingBuffer<uint64_t>(_writeBufferSize);
	_numWriteThread = 10;
	for(uint32_t i = 0; i < _numWriteThread; ++i) {
		_writeThreads.push_back(thread(&FileDataCache::writeBackThread, this));
	}
	_prefetchBufferSize = 10;
	_prefetchBuffer = new RingBuffer<std::pair<uint64_t, uint32_t> >(_prefetchBufferSize);
	_numPrefetchThread = 10;
	for(uint32_t i = 0; i < _numPrefetchThread; ++i) {
		_prefetchThreads.push_back(thread(&FileDataCache::doPrefetch, this));
	}
}

uint32_t FileDataCache::readDataCache(uint64_t segmentId, uint32_t primary, void* buf, uint32_t size, uint32_t offset) {
	_dataCacheMutex.lock();
	debug("Read Cache %" PRIu64 " at %" PRIu32 " for %" PRIu32 "\n",segmentId,offset,size);
	if(_segmentStatus.count(segmentId) == 1) {
		mutex * tempMutex = _segmentLock[segmentId];
		tempMutex->lock();
		_dataCacheMutex.unlock();
		//		if(_segmentStatus[segmentId] == DIRTY) {
		tempMutex->unlock();
		memcpy(buf, _segmentDataCache[segmentId].buf + offset, size);

		//		}
	} else {
		_segmentStatus[segmentId] = CLEAN;
		_segmentPrimary[segmentId] = primary;
		mutex * tempMutex = new mutex();
		_segmentLock[segmentId] = tempMutex;
		tempMutex->lock();
		_dataCacheMutex.unlock();

		uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
		struct SegmentTransferCache segmentTransferCache = client->getSegment(_clientId, sockfd, segmentId);
		struct SegmentData segmentCache;
		segmentCache.buf = segmentTransferCache.buf;
		segmentCache.info.segmentId = segmentId;
		segmentCache.info.segmentSize = segmentTransferCache.length;
		segmentCache.info.segmentPath = "";
		_segmentDataCache[segmentId] = segmentCache;
		tempMutex->unlock();
		memcpy(buf, segmentCache.buf + offset, size);
	}

	updateLru(segmentId);

	return size;

	// Read Cache
	//uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
	//struct SegmentTransferCache segmentCache = client->getSegment(_clientId, sockfd, segmentId);
	//memcpy(buf, segmentCache.buf + offset, size);
	//return size;
}

uint32_t FileDataCache::writeDataCache(uint64_t segmentId, uint32_t primary, const void* buf, uint32_t size, uint32_t offset) {
	_dataCacheMutex.lock();
	// TODO: Check - Forbid Write to Sealed Segment
	struct SegmentData segmentDataCache;
	if(_segmentStatus.count(segmentId) == 0) {
		_segmentPrimary[segmentId] = primary;
		segmentDataCache.info.segmentId = segmentId;
		segmentDataCache.info.segmentSize = 0;
		segmentDataCache.buf = (char*)MemoryPool::getInstance().poolMalloc(_segmentSize); 
		mutex * tempMutex = new mutex();
		_segmentLock[segmentId] = tempMutex;
		//tempMutex->lock();
	} else {
		segmentDataCache = _segmentDataCache[segmentId];
	}
	_dataCacheMutex.unlock();

	// Bound Check
    if (offset + size > _segmentSize) {
        perror ("exceed boundary");
        return -1;
    }

    // Copy new update/data into segmentData struct
	memcpy(segmentDataCache.buf + offset, buf, size);

    // Inline-update: Add offset, length pair
    segmentDataCache.info.offlenVector.push_back(make_pair(offset, size));

	_segmentStatus[segmentId] = DIRTY;
	if(offset + size > segmentDataCache.info.segmentSize)
		segmentDataCache.info.segmentSize = offset + size;

	_segmentDataCache[segmentId] = segmentDataCache;
	updateLru(segmentId);
	return size;
}

void FileDataCache::closeDataCache(uint64_t segmentId, bool sync) {
	_dataCacheMutex.lock();
	if(_segmentStatus.count(segmentId) == 0) {
		_dataCacheMutex.unlock();
		debug("Segment %" PRIu64 " not Cached\n", segmentId);
		return ;
	}

	/*
	   _lruListMutex.lock();
	   list<uint64_t>::iterator tempIt;
	   tempIt = _segment2LruMap[segmentId];
	   _segmentLruList.erase(tempIt);
	   _segment2LruMap.erase(segmentId);
	   _lruListMutex.unlock();
	 */

	debug("Closing Data Cache %" PRIu64 "\n",segmentId);

	// Read Cache
	if(_segmentStatus[segmentId] == CLEAN) {
		mutex * tempMutex = _segmentLock[segmentId];
		tempMutex->lock();
		ClientStorageModule* storageModule = client->getStorageModule();
		storageModule->closeSegment(segmentId);
		tempMutex->unlock();
		_segmentStatus.erase(segmentId);
		_segmentPrimary.erase(segmentId);
		_segmentLock.erase(segmentId);
		_segmentDataCache.erase(segmentId);
		_prefetchBitmap.erase(segmentId);
		_dataCacheMutex.unlock();
		return ;
	}

	//_segmentStatus.erase(segmentId);
	//_segmentLock.erase(segmentId);
	_dataCacheMutex.unlock();
	// Write Cache
	if(sync)
		doWriteBack(segmentId);
	else
		writeBack(segmentId);

	// TODO: Convert to Read Cache
	//_segmentStatus[segmentId] = CLEAN;
}

void FileDataCache::prefetchSegment(uint64_t segmentId, uint32_t primary) {
	_prefetchBitmapMutex.lock();	
	if(_prefetchBitmap.count(segmentId) == 0) {
		_dataCacheMutex.lock();
		if(_segmentStatus.count(segmentId) == 0) {
			_prefetchBitmap[segmentId] = true;
			_prefetchBuffer->push(make_pair(segmentId, primary));
		}
		_dataCacheMutex.unlock();
	}
	_prefetchBitmapMutex.unlock();	
}

void FileDataCache::writeBack(uint64_t segmentId) {
	_writeBuffer->push(segmentId);
}

void FileDataCache::doPrefetch() {
	std::pair<uint64_t, uint32_t> tempPair;
	while(1) {
		tempPair = _prefetchBuffer->pop();
		_dataCacheMutex.lock();
		uint64_t segmentId = tempPair.first;
		uint32_t primary = tempPair.second;
		uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);
		bool fetch = false;

		debug("Prefetch %" PRIu64 "\n",segmentId);

		mutex * tempMutex;
		if(_segmentStatus.count(segmentId) == 0) {
			_segmentStatus[segmentId] = CLEAN;
			_segmentPrimary[segmentId] = primary;
			tempMutex = new mutex();
			_segmentLock[segmentId] = tempMutex;
			fetch = true;
			debug("Create Cache for %" PRIu64 "\n",segmentId);
		} else
			tempMutex = _segmentLock[segmentId];

		tempMutex->lock();
		_dataCacheMutex.unlock();
		struct SegmentTransferCache segmentTransferCache = client->getSegment(_clientId, sockfd, segmentId);
		if(fetch) {
			struct SegmentData segmentCache;
			segmentCache.buf = segmentTransferCache.buf;
			segmentCache.info.segmentId = segmentId;
			segmentCache.info.segmentSize = segmentTransferCache.length;
			segmentCache.info.segmentPath = "";
			_segmentDataCache[segmentId] = segmentCache;
		}
		tempMutex->unlock();
		updateLru(segmentId);

	}
}

void FileDataCache::writeBackThread() {
	while(1) {
		uint64_t segmentId = _writeBuffer->pop();
		doWriteBack(segmentId);
	}
	return ;
}

void FileDataCache::doWriteBack(uint64_t segmentId) {
	_dataCacheMutex.lock();

	if(_segmentStatus.count(segmentId) == 0) {
		_dataCacheMutex.unlock();
		return;
	}

	SegmentStatus segmentStatus = _segmentStatus[segmentId];
	uint32_t primary = _segmentPrimary[segmentId];
	struct SegmentData segmentDataCache = _segmentDataCache[segmentId];
	_segmentPrimary.erase(segmentId);
	_segmentDataCache.erase(segmentId);
	_segmentStatus[segmentId] = WRITEBACK;
	mutex * tempMutex = _segmentLock[segmentId];
	tempMutex->lock();
	_dataCacheMutex.unlock();

	if((segmentDataCache.info.segmentSize == 0) || (segmentStatus != DIRTY)){
		MemoryPool::getInstance().poolFree(segmentDataCache.buf);
		_segmentStatus.erase(segmentId);
		_segmentLock.erase(segmentId);
		tempMutex->unlock();
        // Delete the tempMutex before return
        delete tempMutex;
		return;
	}

	uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);

	unsigned char checksum[MD5_DIGEST_LENGTH];
	memset (checksum, 0, MD5_DIGEST_LENGTH);

#ifdef USE_CHECKSUM
	MD5((unsigned char*) segmentDataCache.buf, segmentDataCache.info.segmentSize, checksum);
#endif

	debug("Write Back Segment %" PRIu64 " ,Size %" PRIu32 "\n", segmentId, segmentDataCache.info.segmentSize);
	_clientCommunicator->sendSegment(_clientId, sockfd, segmentDataCache, _codingScheme, _codingSetting, md5ToHex(checksum));
	MemoryPool::getInstance().poolFree(segmentDataCache.buf);
	_segmentStatus.erase(segmentId);
	_segmentLock.erase(segmentId);
	tempMutex->unlock();
    // Delete the tempMutex before return
    delete tempMutex;
	return ;
}

void FileDataCache::updateLru(uint64_t segmentId) {
	_lruListMutex.lock();
	list<uint64_t>::iterator tempIt;
	if(_segment2LruMap.count(segmentId) == 1){
		tempIt = _segment2LruMap[segmentId];
		_segmentLruList.splice(_segmentLruList.end(), _segmentLruList, tempIt);
	}
	// Create LRU Record
	else {
		if(_segmentLruList.size() >= _lruSizeLimit) {
			uint64_t segmentToClose = _segmentLruList.front();
			_segmentLruList.pop_front();
			_segment2LruMap.erase(segmentToClose);
			closeDataCache(segmentToClose);
		}
		tempIt = _segmentLruList.insert(_segmentLruList.end(), segmentId);
		_segment2LruMap[segmentId] = tempIt;
	}
	_lruListMutex.unlock();
	return ;

}
