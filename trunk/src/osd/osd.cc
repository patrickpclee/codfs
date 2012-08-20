/**
 * osd.cc
 */

#include <signal.h>
#include <thread>
#include <stdio.h>
#include <vector>
#include "osd.hh"
#include "segmentlocation.hh"
#include "../common/debug.hh"
#include "../common/metadata.hh"
#include "../config/config.hh"
#include "../common/garbagecollector.hh"
#include "../protocol/status/osdstartupmsg.hh"
#include "../protocol/status/osdshutdownmsg.hh"
#include "../protocol/status/osdstatupdatereplymsg.hh"

// for random srand() time() rand()
#include <stdlib.h>
#include <time.h>

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT)
		exit(42);
}

/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

mutex pendingObjectChunkMutex;
mutex pendingSegmentChunkMutex;
mutex pendingSegmentCountMutex;
mutex receivedSegmentsMutex;
;
mutex codingSettingMapMutex;

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

	struct ObjectData objectData = { };

	// TODO: check if OSD has object
	// TODO: check if osd list exists in cache

	// 1. ask MDS to get object information

	ObjectTransferOsdInfo objectInfo = _osdCommunicator->getObjectInfoRequest(
			objectId);
	const CodingScheme codingScheme = objectInfo._codingScheme;
	const string codingSetting = objectInfo._codingSetting;

	// check which segments are needed to request
	vector<uint32_t> requiredSegments = _codingModule->getRequiredSegmentIds(
			codingScheme, codingSetting);

	debug("[Download] ObjectSize = %" PRIu64 "\n", objectInfo._size);

	// 2. initialize list and count

	const uint32_t segmentCount = requiredSegments.size();
	{
		lock_guard<mutex> lk(pendingSegmentCountMutex);
		_pendingSegmentCount[objectId] = segmentCount;
		debug("PendingSegmentCount = %" PRIu32 "\n", segmentCount);
	}

	receivedSegmentsMutex.lock();
	_receivedSegments[objectId] = vector<struct SegmentData>(segmentCount);
	vector<struct SegmentData>& segmentDataList = _receivedSegments[objectId];
	receivedSegmentsMutex.unlock();

	debug("[Download] Reserve %" PRIu32 " segments, segmentDataList = %zu\n",
			segmentCount, segmentDataList.size());

	// 3. request segments (either from disk or wait segments to arrive)

	for (uint32_t requiredSegmentIndex : requiredSegments) {

		uint32_t osdId = objectInfo._osdList[requiredSegmentIndex];

		if (osdId == _osdId) {
			// read segment from disk
			struct SegmentData segmentData = _storageModule->readSegment(
					objectId, requiredSegmentIndex, 0);
			segmentDataList[requiredSegmentIndex] = segmentData;

			{
				lock_guard<mutex> lk(pendingSegmentCountMutex);
				_pendingSegmentCount[objectId]--;
				debug("%s\n", "Read from local segment");
			}

		} else {
			// request segment from other OSD
			debug("sending request for segment %" PRIu32 "\n",
					requiredSegmentIndex);
			_osdCommunicator->getSegmentRequest(osdId, objectId,
					requiredSegmentIndex);
		}
	}

	// 4. wait until all segments have arrived

	while (1) {
		uint32_t segmentLeft = 0;
		{
			lock_guard<mutex> lk(pendingSegmentCountMutex);
			segmentLeft = _pendingSegmentCount[objectId];
		}

		if (segmentLeft == 0) {
			// 5. decode segments

			debug(
					"[DOWNLOAD] Start Decoding with %d scheme and settings = %s\n",
					(int)codingScheme, codingSetting.c_str());
			objectData = _codingModule->decodeSegmentToObject(codingScheme,
					objectId, segmentDataList, codingSetting);

			// debug
//			_storageModule->writeObject (12345, objectData.buf, 0, objectData.info.objectSize);

			break;
		} else {
			usleep(100000); // 0.1s
		}
	}

	debug("%s\n", "[DOWNLOAD] Send Object");

	// 5. send object (to be implemented)
	_osdCommunicator->sendObject(sockfd, objectData);

	// DEBUG, write to disk
	/*
	_storageModule->createAndOpenObject(123456, objectData.info.objectSize);
	_storageModule->writeObject(123456, objectData.buf, 0,
			objectData.info.objectSize);
	*/

	// clean up

	for (auto segment : segmentDataList) {
		MemoryPool::getInstance().poolFree(segment.buf);
	}

	{
		lock_guard<mutex> lk(receivedSegmentsMutex);
		_receivedSegments.erase(objectId);
	}

	{
		lock_guard<mutex> lk(pendingSegmentCountMutex);
		_pendingSegmentCount.erase(objectId);
	}

	debug("%s\n", "[DOWNLOAD] Cleanup completed");

}

void Osd::getSegmentRequestProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData = _storageModule->readSegment(objectId,
			segmentId, 0);
	_osdCommunicator->sendSegment(sockfd, segmentData);
}

void Osd::putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount,
		CodingScheme codingScheme, string setting) {

	struct CodingSetting codingSetting;
	codingSetting.codingScheme = codingScheme;
	codingSetting.setting = setting;

	// initialize chunkCount value
	{
		lock_guard<mutex> lk(pendingObjectChunkMutex);
		_pendingObjectChunk[objectId] = chunkCount;
	}

	{
		lock_guard<mutex> lk(codingSettingMapMutex);
		_codingSettingMap[objectId] = codingSetting;
	}

	// create object and cache
	_storageModule->createObject(objectId, length);
	_osdCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

}

void Osd::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	// TODO: check integrity of object received
	bool chunkRemaining = false;

	while (1) {

		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
			chunkRemaining = (bool) _pendingObjectChunk.count(objectId);
		}

		if (!chunkRemaining) {
			// if all chunks have arrived, send ack
			_osdCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);
			break;
		} else {
			usleep(100000); // sleep 0.1s
		}

	}

}

void Osd::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	// TODO: check integrity of segment received
	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);
	bool chunkRemaining = false;

	while (1) {

		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
			chunkRemaining = (bool) _pendingSegmentChunk.count(segmentKey);
		}

		if (!chunkRemaining) {
			// if all chunks have arrived, send ack
			_osdCommunicator->replyPutSegmentEnd(requestId, sockfd, objectId,
					segmentId);
			break;
		} else {
			usleep(100000); // sleep 0.1s
		}

	}

}

uint32_t Osd::putObjectDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeObjectCache(objectId, buf, offset,
			length);

	uint32_t chunkLeft = 0;
	{
		lock_guard<mutex> lk(pendingObjectChunkMutex);
		// update pendingObjectChunk value
		_pendingObjectChunk[objectId]--;
		chunkLeft = _pendingObjectChunk[objectId];
	}

	// if all chunks have arrived
	if (chunkLeft == 0) {
		struct ObjectCache objectCache = _storageModule->getObjectCache(
				objectId);

		// write cache to disk
		byteWritten = _storageModule->writeObject(objectId, objectCache.buf, 0,
				objectCache.length);

		// perform coding
		struct CodingSetting codingSetting;
		{
			lock_guard<mutex> lk(codingSettingMapMutex);
			codingSetting = _codingSettingMap[objectId];
			_codingSettingMap.erase(objectId);
		}

		debug("Coding Scheme = %d setting = %s\n",
				(int) codingSetting.codingScheme, codingSetting.setting.c_str());

		vector<struct SegmentData> segmentDataList =
				_codingModule->encodeObjectToSegment(codingSetting.codingScheme,
						objectId, objectCache.buf, objectCache.length,
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

		// close file and free cache
		_storageModule->closeObject(objectId);

		// remove from map
		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
			_pendingObjectChunk.erase(objectId);
		}

		// Acknowledge MDS for Object Upload Completed
		_osdCommunicator->objectUploadAck(objectId, codingSetting.codingScheme,
				codingSetting.setting, nodeList);

	}

	return byteWritten;
}

void Osd::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t length,
		uint32_t chunkCount) {

	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);
	bool isDownload = false;
	{
		lock_guard<mutex> lk(pendingSegmentCountMutex);
		isDownload = (bool) _pendingSegmentCount.count(objectId);
	}

	debug(
			"[PUT_SEGMENT_INIT] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 "isDownload = %d\n",
			objectId, segmentId, length, chunkCount, isDownload);

	// initialize chunkCount value
	{
		lock_guard<mutex> lk(pendingSegmentChunkMutex);
		_pendingSegmentChunk[segmentKey] = chunkCount;
	}

	if (isDownload) {
		receivedSegmentsMutex.lock();
		struct SegmentData& segmentData = _receivedSegments[objectId][segmentId];
		receivedSegmentsMutex.unlock();

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
	bool isDownload = false;

	{
		lock_guard<mutex> lk(pendingSegmentCountMutex);
		isDownload = (bool) _pendingSegmentCount.count(objectId);
	}

	// if the segment received is for download process
	if (isDownload) {
		receivedSegmentsMutex.lock();
		struct SegmentData& segmentData = _receivedSegments[objectId][segmentId];
		receivedSegmentsMutex.unlock();

		debug("offset = %" PRIu32 ", length = %" PRIu32 "\n", offset, length);
		memcpy(segmentData.buf + offset, buf, length);
	} else {
		byteWritten = _storageModule->writeSegment(objectId, segmentId, buf,
				offset, length);
	}

	uint32_t chunkLeft = 0;
	{
		lock_guard<mutex> lk(pendingSegmentChunkMutex);
		// update pendingSegmentChunk value
		_pendingSegmentChunk[segmentKey]--;
		chunkLeft = _pendingSegmentChunk[segmentKey];
	}

	// if all chunks have arrived
	if (chunkLeft == 0) {

		if (!isDownload) {
			// close file and free cache
			_storageModule->closeSegment(objectId, segmentId);
		} else {
			{
				lock_guard<mutex> lk(pendingSegmentCountMutex);
				_pendingSegmentCount[objectId]--;
			}
			debug("all chunks for segment %" PRIu32 "is received\n", segmentId);
		}

		// remove from map
		{
			lock_guard<mutex> lk(pendingSegmentChunkMutex);
			_pendingSegmentChunk.erase(segmentKey);
		}

	}

	return byteWritten;
}

void Osd::recoveryProcessor(uint32_t requestId, uint32_t sockfd) {
	// TODO: recovery to be implemented
}

void Osd::OsdStatUpdateRequestProcessor(uint32_t requestId, uint32_t sockfd) {
	OsdStatUpdateReplyMsg* replyMsg = new OsdStatUpdateReplyMsg(
			_osdCommunicator, sockfd, _osdId, rand() % 100, rand() % 200);
	replyMsg->prepareProtocolMsg();
	_osdCommunicator->addMessage(replyMsg);
}

OsdCommunicator* Osd::getCommunicator() {
	return _osdCommunicator;
}

/*
 SegmentLocationCache* Osd::getSegmentLocationCache() {
 return _segmentLocationCache;
 }
 */

uint32_t Osd::getOsdId() {
	return _osdId;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	osd->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	// wait for message
	communicator->waitForMessage();

}

void startTestThread(Communicator* communicator) {

	/*
	 printf("HEHE\n");
	 OsdStartupMsg* testmsg = new OsdStartupMsg(communicator,
	 communicator->getMonitorSockfd(), osd->getOsdId(), rand() % 10,
	 rand() % 10);
	 printf("Prepared msg\n");
	 testmsg->prepareProtocolMsg();
	 communicator->addMessage(testmsg);
	 printf("Prepared add \n");
	 sleep(120);
	 OsdShutdownMsg* msg = new OsdShutdownMsg(communicator,
	 communicator->getMonitorSockfd(), osd->getOsdId());
	 msg->prepareProtocolMsg();
	 communicator->addMessage(msg);
	 printf("DONE\n");
	 */

}

/**
 * Main function
 * @return 0 if success;
 */

int main(int argc, char* argv[]) {

	signal(SIGINT, sighandler);

	string configFilePath;

	if (argc < 2) {
		cout << "Usage: ./OSD [OSD CONFIG FILE]" << endl;
		exit(0);
	} else {
		configFilePath = string(argv[1]);
	}

	// create new OSD object and communicator
	osd = new Osd(configFilePath);

	// create new communicator
	OsdCommunicator* communicator = osd->getCommunicator();

	// set identity
	communicator->setId(osd->getOsdId());
	communicator->setComponentType(OSD);

	// create server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	communicator->connectAllComponents();

	debug("%s\n", "starting test thread");
	sleep(5);

	if (osd->getOsdId() == 52000) {
//		osd->getObjectRequestProcessor(0, 0, 83937998);
	}

	thread testThread(startTestThread, communicator);
	// TODO: pause before connect for now
	//getchar();

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();
//	testThread.join();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}
