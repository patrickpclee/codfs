/**
 * osd.cc
 */

#include <thread>
#include <vector>
#include <openssl/md5.h>
#include "osd.hh"
#include "segmentlocation.hh"
#include "../common/debug.hh"
#include "../common/metadata.hh"
#include "../config/config.hh"
#include "../protocol/status/osdstartupmsg.hh"
#include "../protocol/status/osdshutdownmsg.hh"
#include "../protocol/status/osdstatupdatereplymsg.hh"

// for random srand() time() rand() getloadavg()
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/statvfs.h>

#define INF (1<<29)
#define DISK_PATH "/"

/// Osd Object
extern Osd* osd;

/// Config Object
extern ConfigLayer* configLayer;

/*
 mutex requestedSegmentsMutex;
 mutex initializationMutex;
 mutex cleanupMutex;
 */
mutex objectRequestCountMutex;

Osd::Osd(string configFilePath) {

	configLayer = new ConfigLayer(configFilePath.c_str(), "common.xml");
	//segmentLocationCache = new SegmentLocationCache();
	_storageModule = new StorageModule();
	_osdCommunicator = new OsdCommunicator();
	_codingModule = new CodingModule();

	_osdId = configLayer->getConfigInt("Osdid");

	srand(time(NULL)); //random test
}

Osd::~Osd() {
	//delete _segmentLocationCache;
	delete _storageModule;
	delete _osdCommunicator;
}

/**
 * Save the received OSD list into the SegmentLocationCache
 * 1. If existing list is found, delete it
 * 2. Insert the new list into the cache
 */

uint32_t Osd::osdListProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, vector<SegmentLocation> osdList) {

	/*
	 _segmentLocationCache->deleteSegmentLocation(objectId);
	 _segmentLocationCache->writeSegmentLocation(objectId, osdList);
	 */

	return 0;
}

/**
 * Send the object to the target
 */

void Osd::getObjectRequestProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	objectRequestCountMutex.lock();
	if (!_objectRequestCount.count(objectId)) {
		_objectRequestCount.set(objectId, 1);
		mutex* tempMutex = new mutex();
		_objectDownloadMutex.set(objectId, tempMutex);
		_objectDataMap.set(objectId, { });
		_isObjectDownloaded.set(objectId, false);
	} else {
		_objectRequestCount.increment(objectId);
	}
	struct ObjectData& objectData = _objectDataMap.get(objectId);
	objectRequestCountMutex.unlock();

	{

		debug("Before Lock for objectId = %" PRIu64 "\n", objectId);
		lock_guard<mutex> lk(*(_objectDownloadMutex.get(objectId)));
		debug("Inside Lock for objectId = %" PRIu64 "\n", objectId);

		if (!_isObjectDownloaded.get(objectId)) {

			if (_storageModule->isObjectCached(objectId)) {
				// case 1: if object exists in cache
				debug ("Object ID = %" PRIu64 " exists in cache", objectId);
				objectData = _storageModule->getObjectFromDiskCache(objectId);
			} else {
				// case 2: if object does not exist in cache

				// TODO: check if osd list exists in cache

				// 1. ask MDS to get object information

				ObjectTransferOsdInfo objectInfo =
						_osdCommunicator->getObjectInfoRequest(objectId);
				const CodingScheme codingScheme = objectInfo._codingScheme;
				const string codingSetting = objectInfo._codingSetting;

				// check which segments are needed to request
				vector<uint32_t> requiredSegments =
						_codingModule->getRequiredSegmentIds(codingScheme,
								codingSetting);
				uint32_t totalNumOfSegments =
						_codingModule->getNumberOfSegments(codingScheme,
								codingSetting);

				// 2. initialize list and count

				const uint32_t segmentCount = requiredSegments.size();

				_downloadSegmentRemaining.set(objectId, segmentCount);
				_receivedSegmentData.set(objectId,
						vector<struct SegmentData>(totalNumOfSegments));
				vector<struct SegmentData>& segmentDataList =
						_receivedSegmentData.get(objectId);

				debug("PendingSegmentCount = %" PRIu32 "\n", segmentCount);

				// 3. request segments
				// case 1: load from disk
				// case 2: request from OSD
				// case 3: already requested

				for (uint32_t i : requiredSegments) {

					uint32_t osdId = objectInfo._osdList[i];

					if (osdId == _osdId) {
						// read segment from disk
						struct SegmentData segmentData =
								_storageModule->readSegment(objectId, i, 0);

						// segmentDataList only reserved space for the requiredSegments
						segmentDataList[i] = segmentData;

						_downloadSegmentRemaining.decrement(objectId);
						debug(
								"Read from local segment for Object ID = %" PRIu64 " Segment ID = %" PRIu32 "\n",
								objectId, i);

					} else {
						// request segment from other OSD
						debug("sending request for segment %" PRIu32 "\n", i);
						_osdCommunicator->getSegmentRequest(osdId, objectId, i);
					}
				}

				// 4. wait until all segments have arrived

				while (1) {
					if (_downloadSegmentRemaining.get(objectId) == 0) {
						// 5. decode segments

						debug(
								"[DOWNLOAD] Start Decoding with %d scheme and settings = %s\n",
								(int)codingScheme, codingSetting.c_str());
						objectData = _codingModule->decodeSegmentToObject(
								codingScheme, objectId, segmentDataList,
								codingSetting);

						// clean up segment data
						_downloadSegmentRemaining.erase(objectId);

						for (uint32_t i : requiredSegments) {
							debug(
									"%" PRIu32 " free segment %" PRIu32 " addr = %p\n",
									i, segmentDataList[i].info.segmentId, segmentDataList[i].buf);
							MemoryPool::getInstance().poolFree(
									segmentDataList[i].buf);
							debug("%" PRIu32 " segment %" PRIu32 " free-d\n",
									i, segmentDataList[i].info.segmentId);
						}
						_receivedSegmentData.erase(objectId);

						break;
					} else {
						usleep(50000); // 0.01s
					}
				}
			}

			debug("%s\n", "[DOWNLOAD] Send Object");

			_isObjectDownloaded.set(objectId, true);
		}
	}

	// 5. send object
	_osdCommunicator->sendObject(_osdId, sockfd, objectData);

	{
		lock_guard<mutex> lk(objectRequestCountMutex);
		_objectRequestCount.decrement(objectId);
		if (_objectRequestCount.get(objectId) == 0) {
			_objectRequestCount.erase(objectId);

			// cache objectData
			struct ObjectTransferCache objectTransferCache;
			objectTransferCache.buf = objectData.buf;
			objectTransferCache.length = objectData.info.objectSize;
			_storageModule->saveObjectToDisk(objectId, objectTransferCache);

			// free objectData
			debug("free object %" PRIu64 "\n", objectId);
			MemoryPool::getInstance().poolFree(objectData.buf);
			debug("object %" PRIu64 "free-d\n", objectId);

			_objectDataMap.erase(objectId);

			debug("%s\n", "[DOWNLOAD] Cleanup completed");
		}
	}

}

void Osd::getSegmentRequestProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData = _storageModule->readSegment(objectId,
			segmentId, 0);
	_osdCommunicator->sendSegment(sockfd, segmentData);
	MemoryPool::getInstance().poolFree(segmentData.buf);
	debug("Segment ID = %" PRIu32 " free-d\n", segmentId);
}

void Osd::putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount,
		CodingScheme codingScheme, string setting, string checksum) {

	struct CodingSetting codingSetting;
	codingSetting.codingScheme = codingScheme;
	codingSetting.setting = setting;

	// initialize chunkCount value
	_pendingObjectChunk.set(objectId, chunkCount);

	// save coding setting
	_codingSettingMap.set(objectId, codingSetting);

	// create object and cache
	_storageModule->createObjectCache(objectId, length);
	_osdCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

	// save md5 to map
	_checksumMap.set(objectId, checksum);

}

void Osd::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	// TODO: check integrity of object received
	while (1) {

		if (_pendingObjectChunk.get(objectId) == 0) {
			// if all chunks have arrived
			struct ObjectTransferCache objectCache =
					_storageModule->getObjectCache(objectId);

			// compute md5 checksum
			unsigned char checksum[MD5_DIGEST_LENGTH];
			MD5((unsigned char*) objectCache.buf, objectCache.length, checksum);
			debug_cyan ("md5 of object ID %" PRIu64 " = %s\n", objectId, md5ToHex(checksum).c_str());

			// compare md5 with saved one
			if (_checksumMap.get(objectId) != md5ToHex(checksum)) {
				debug ("MD5 of Object ID = %" PRIu64 " mismatch!\n", objectId);
				exit (-1);
			} else {
				debug ("MD5 of Object ID = %" PRIu64 " match\n", objectId);
				_checksumMap.erase(objectId);
			}

			// perform coding
			struct CodingSetting codingSetting = _codingSettingMap.get(
					objectId);
			_codingSettingMap.erase(objectId);

			debug("Coding Scheme = %d setting = %s\n",
					(int) codingSetting.codingScheme, codingSetting.setting.c_str());

			vector<struct SegmentData> segmentDataList =
					_codingModule->encodeObjectToSegment(
							codingSetting.codingScheme, objectId,
							objectCache.buf, objectCache.length,
							codingSetting.setting);

			// request secondary OSD list
			vector<struct SegmentLocation> segmentLocationList =
					_osdCommunicator->getOsdListRequest(objectId, MONITOR,
							segmentDataList.size());

			vector<uint32_t> nodeList;
			uint32_t i = 0;
			for (const auto segmentData : segmentDataList) {

				// if destination is myself
				if (segmentLocationList[i].osdId == _osdId) {
					_storageModule->createSegment(objectId,
							segmentData.info.segmentId,
							segmentData.info.segmentSize);
					_storageModule->writeSegment(objectId,
							segmentData.info.segmentId, segmentData.buf, 0,
							segmentData.info.segmentSize);
					_storageModule->closeSegment(objectId,
							segmentData.info.segmentId);
				} else {
					uint32_t dstSockfd = _osdCommunicator->getSockfdFromId(
							segmentLocationList[i].osdId);
					_osdCommunicator->sendSegment(dstSockfd, segmentData);
				}

				nodeList.push_back(segmentLocationList[i].osdId);

				// free memory
				MemoryPool::getInstance().poolFree(segmentData.buf);

				i++;
			}

			_pendingObjectChunk.erase(objectId);

			// Acknowledge MDS for Object Upload Completed
			_osdCommunicator->objectUploadAck(objectId,
					codingSetting.codingScheme, codingSetting.setting,
					nodeList, md5ToHex(checksum));

			cout << "Object " << objectId << " uploaded" << endl;

			// if all chunks have arrived, send ack
			_osdCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);

			// after ack, write object cache to disk
			_storageModule->saveObjectToDisk(objectId, objectCache);

			// close file and free cache
			_storageModule->closeObjectCache(objectId);

			break;
		} else {
			usleep(10000); // sleep 0.01s
		}

	}

}

void Osd::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	// TODO: check integrity of segment received
	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);

	while (1) {

		// if all chunks have arrived, send ack
		if (!_pendingSegmentChunk.count(segmentKey)) {
			_osdCommunicator->replyPutSegmentEnd(requestId, sockfd, objectId,
					segmentId);
			break;
		} else {
			usleep(10000); // sleep 0.01s
		}

	}

}

uint32_t Osd::putObjectDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeObjectCache(objectId, buf, offset,
			length);

	_pendingObjectChunk.decrement(objectId);

	return byteWritten;
}

void Osd::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t length,
		uint32_t chunkCount) {

	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);
	bool isDownload = _downloadSegmentRemaining.count(objectId);

	debug(
			"[PUT_SEGMENT_INIT] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 "isDownload = %d\n",
			objectId, segmentId, length, chunkCount, isDownload);

	_pendingSegmentChunk.set(segmentKey, chunkCount);

	if (isDownload) {
		struct SegmentData& segmentData =
				_receivedSegmentData.get(objectId)[segmentId];

		segmentData.info.objectId = objectId;
		segmentData.info.segmentId = segmentId;
		segmentData.info.segmentSize = length;
		segmentData.buf = MemoryPool::getInstance().poolMalloc(length);
	} else {
		// create file
		_storageModule->createSegment(objectId, segmentId, length);
	}

	_osdCommunicator->replyPutSegmentInit(requestId, sockfd, objectId,
			segmentId);

}

uint32_t Osd::putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t offset, uint32_t length,
		char* buf) {

	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);

	uint32_t byteWritten = 0;
	bool isDownload = _downloadSegmentRemaining.count(objectId);

	// if the segment received is for download process
	if (isDownload) {
		struct SegmentData& segmentData =
				_receivedSegmentData.get(objectId)[segmentId];

		debug("offset = %" PRIu32 ", length = %" PRIu32 "\n", offset, length);
		memcpy(segmentData.buf + offset, buf, length);
	} else {
		byteWritten = _storageModule->writeSegment(objectId, segmentId, buf,
				offset, length);
	}

	_pendingSegmentChunk.decrement(segmentKey);

	// if all chunks have arrived
	if (_pendingSegmentChunk.get(segmentKey) == 0) {

		if (!isDownload) {
			// close file and free cache
			_storageModule->closeSegment(objectId, segmentId);
		} else {
			_downloadSegmentRemaining.decrement(objectId);
			debug("all chunks for segment %" PRIu32 "is received\n", segmentId);
		}

		// remove from map
		_pendingSegmentChunk.erase(segmentKey);

	}

	return byteWritten;
}

void Osd::recoveryProcessor(uint32_t requestId, uint32_t sockfd) {
	// TODO: recovery to be implemented
}

void Osd::OsdStatUpdateRequestProcessor(uint32_t requestId, uint32_t sockfd) {
	OsdStatUpdateReplyMsg* replyMsg = new OsdStatUpdateReplyMsg(
			_osdCommunicator, sockfd, _osdId, getFreespace(), getCpuLoadavg(2));
	replyMsg->prepareProtocolMsg();
	_osdCommunicator->addMessage(replyMsg);
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
//		printf("Disk %s: \n", DISK_PATH);
//		printf("\tblock size: %u\n", fiData.f_bsize);
//		printf("\ttotal no blocks: %i\n", fiData.f_blocks);
//		printf("\tfree blocks: %i\n", fiData.f_bfree);
		return ((uint32_t) (fiData.f_bsize * fiData.f_bfree / 1024 / 1024));
	}
}

OsdCommunicator* Osd::getCommunicator() {
	return _osdCommunicator;
}

StorageModule* Osd::getStorageModule() {
	return _storageModule;
}

/*
 SegmentLocationCache* Osd::getSegmentLocationCache() {
 return _segmentLocationCache;
 }
 */

uint32_t Osd::getOsdId() {
	return _osdId;
}

/*
 bool Osd::isSegmentRequested(uint64_t objectId, uint32_t segmentId) {
 lock_guard<mutex> lk(requestedSegmentsMutex);
 bool requested = _requestedSegments.get(objectId)[segmentId];

 // if segment is already requested, return true
 if (requested) {
 return true;
 }

 // else, ask this thread to send request and set it to true
 _requestedSegments.get(objectId)[segmentId] = true;
 return false;
 }
 */

