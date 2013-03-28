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
}

uint32_t FileDataCache::readDataCache(uint64_t segmentId, uint32_t primary, void* buf, uint32_t size, uint32_t offset) {
	// Check Write Cache
	if(_segmentStatus.count(segmentId) == 1) {
		if(_segmentStatus[segmentId] == DIRTY) {
			memcpy(buf, _segmentDataCache[segmentId].buf + offset, size);
			return size;
		}
	} else {
		_segmentStatus[segmentId] = CLEAN;
		_segmentPrimary[segmentId] = primary;
	}
	
	// Read Cache
	uint32_t sockfd = _clientCommunicator->getSockfdFromId(primary);;
	struct SegmentTransferCache segmentCache = client->getSegment(_clientId, sockfd, segmentId);
	memcpy(buf, segmentCache.buf + offset, size);
	return size;
}

uint32_t FileDataCache::writeDataCache(uint64_t segmentId, uint32_t primary, const void* buf, uint32_t size, uint32_t offset) {
	// TODO: Check - Forbid Write to Sealed Segment
	struct SegmentData segmentDataCache;
	if(_segmentStatus.count(segmentId) == 0) {
		_segmentPrimary[segmentId] = primary;
		segmentDataCache.info.segmentId = segmentId;
		segmentDataCache.info.segmentSize = 0;
		segmentDataCache.buf = (char*)MemoryPool::getInstance().poolMalloc(_segmentSize); 
	} else {
		segmentDataCache = _segmentDataCache[segmentId];
	}

	// TODO: Bound Check
	memcpy(segmentDataCache.buf + offset, buf, size);

	_segmentStatus[segmentId] = DIRTY;
	if(offset + size > segmentDataCache.info.segmentSize)
		segmentDataCache.info.segmentSize = offset + size;

	_segmentDataCache[segmentId] = segmentDataCache;
	return size;
}

void FileDataCache::closeDataCache(uint64_t segmentId) {
	if(_segmentStatus.count(segmentId) == 0) {
		debug("Segment %" PRIu64 " not Cached\n", segmentId);
		return ;
	}
	
	// Read Cache
	if(_segmentStatus[segmentId] == CLEAN) {
		ClientStorageModule* storageModule = client->getStorageModule();
		storageModule->closeSegment(segmentId);
		_segmentStatus.erase(segmentId);
		_segmentPrimary.erase(segmentId);
		return ;
	}

	// Write Cache
	writeBack(segmentId);
	_segmentStatus.erase(segmentId);
	_segmentPrimary.erase(segmentId);
	_segmentDataCache.erase(segmentId);
	// TODO: Convert to Read Cache
	//_segmentStatus[segmentId] = CLEAN;
}

void FileDataCache::writeBack(uint64_t segmentId) {
	SegmentStatus segmentStatus = _segmentStatus[segmentId];
	uint32_t primary = _segmentPrimary[segmentId];
	struct SegmentData segmentDataCache = _segmentDataCache[segmentId];

	if((segmentStatus == CLEAN) || (segmentDataCache.info.segmentSize == 0)){
		MemoryPool::getInstance().poolFree(segmentDataCache.buf);
		return ;
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
	return ;
}
