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

void Osd::getObjectProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	/*

	 vector<struct SegmentData> segmentDataList;
	 vector<struct SegmentLocation> osdIdList;
	 struct ObjectData objectData;

	 if (_storageModule->isObjectExist(objectId)) {
	 // if object exists in cache
	 objectData = _storageModule->readObject(objectId, 0);
	 } else {
	 // get osdIDList from cache, if failed update it from MDS
	 try {
	 osdIdList = _segmentLocationCache->readSegmentLocation(objectId);
	 } catch (CacheMissException &e) {
	 osdIdList = _osdCommunicator->getOsdListRequest(objectId, MDS);
	 _segmentLocationCache->writeSegmentLocation(objectId, osdIdList);
	 }

	 // get segments from the OSD one by one
	 vector<struct SegmentLocation>::const_iterator p;

	 for (p = osdIdList.begin(); p != osdIdList.end(); ++p) {
	 // memory of SegmentData is allocated in getSegmentRequest
	 struct SegmentData segmentData;
	 uint32_t osdId = (*p).osdId;
	 uint32_t segmentId = (*p).segmentId;

	 if (_osdId == osdId) {
	 // case 1: local segment
	 segmentData = _storageModule->readSegment(objectId, segmentId);
	 } else {
	 // case 2: foreign segment
	 segmentData = _osdCommunicator->getSegmentRequest(osdId,
	 objectId, segmentId);
	 }
	 segmentDataList.push_back(segmentData);
	 }

	 // memory of objectData is allocated in decodeSegmentToObject
	 // TODO: decodeSegmentToObject should free memory in segmentDataList
	 objectData = _codingModule->decodeSegmentToObject(objectId,
	 segmentDataList);
	 }

	 _osdCommunicator->sendObject(sockfd, objectData);
	 */

	return;
}

void Osd::getSegmentProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	struct SegmentData segmentData;
	segmentData = _storageModule->readSegment(objectId, segmentId);
	_osdCommunicator->sendSegment(_osdId, sockfd, segmentData);

	return;
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
				_osdCommunicator->sendSegment(_osdId, dstSockfd, segmentData);
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
//		_osdCommunicator->objectUploadAck(objectId, codingSetting.codingScheme,
//		codingSetting. settingnodeList);

	}

	return byteWritten;
}

void Osd::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t length,
		uint32_t chunkCount) {

	debug(
			"[PUT_SEGMENT_INIT] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 ", Length = %" PRIu32 ", Count = %" PRIu32 "\n",
			objectId, segmentId, length, chunkCount);
	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);

	// initialize chunkCount value
	{
		lock_guard<mutex> lk(pendingSegmentChunkMutex);
		_pendingSegmentChunk[segmentKey] = chunkCount;
	}

	// create object and cache
	_storageModule->createSegment(objectId, segmentId, length);
	_osdCommunicator->replyPutSegmentInit(requestId, sockfd, objectId,
			segmentId);

}

uint32_t Osd::putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t offset, uint32_t length,
		char* buf) {

	const string segmentKey = to_string(objectId) + "." + to_string(segmentId);

	uint32_t byteWritten = _storageModule->writeSegment(objectId, segmentId,
			buf, offset, length);

	uint32_t chunkLeft = 0;
	{
		lock_guard<mutex> lk(pendingSegmentChunkMutex);
		// update pendingSegmentChunk value
		_pendingSegmentChunk[segmentKey]--;
		chunkLeft = _pendingSegmentChunk[segmentKey];
	}

	// if all chunks have arrived
	if (chunkLeft == 0) {

		// close file and free cache
		_storageModule->closeSegment(objectId, segmentId);

		// remove from map
		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
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
