/**
 * osd.cc
 */

#include <thread>
#include <stdio.h>
#include <vector>
#include "osd.hh"
#include "segmentlocationcache.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../common/garbagecollector.hh"

/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

// Global Mutex for locking _pendingObjectChunk
mutex pendingObjectChunkMutex;

Osd::Osd(string configFilePath) {
	configLayer = new ConfigLayer(configFilePath.c_str(), "common.xml");
	_segmentLocationCache = new SegmentLocationCache();
	_storageModule = new StorageModule();
	_osdCommunicator = new OsdCommunicator();
	_codingModule = new CodingModule();

	_osdId = configLayer->getConfigInt("Osdid");
}

Osd::~Osd() {
	delete _segmentLocationCache;
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

	_segmentLocationCache->deleteSegmentLocation(objectId);
	_segmentLocationCache->writeSegmentLocation(objectId, osdList);

	return 0;
}

/**
 * Send the object to the target
 */

void Osd::getObjectProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

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

	return;
}

void Osd::getSegmentProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	struct SegmentData segmentData;
	segmentData = _storageModule->readSegment(objectId, segmentId);
	_osdCommunicator->sendSegment(sockfd, segmentData);

	return;
}

void Osd::putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount) {

	// initialize chunkCount value
	pendingObjectChunkMutex.lock();
	_pendingObjectChunk[objectId] = chunkCount;
	pendingObjectChunkMutex.unlock();

	// create object and cache
	_storageModule->createObject(objectId, length);
	_osdCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

}

void Osd::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	// TODO: check integrity of object received
	// TODO: now reply message is actually sent before all data is flushed to disk

	_osdCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);

}

void Osd::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	// TODO: check integrity of segment received
	// TODO: now reply message is actually sent before all data is flushed to disk

	_osdCommunicator->replyPutSegmentEnd(requestId, sockfd, objectId,
			segmentId);

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
		debug("%s\n", "flushing cache to disk");
		byteWritten = _storageModule->writeObject(objectId, objectCache.buf, 0,
				objectCache.length);

		// perform coding
		debug("%s\n", "performing coding");
		vector<struct SegmentData> segmentData =
				_codingModule->encodeObjectToSegment(objectId, objectCache.buf,
						objectCache.length);

		// DEBUG: write segments to disk
		debug("%s\n", "writing segments to disk");
		for (struct SegmentData segment : segmentData) {
			_storageModule->createAndOpenSegment(objectId,
					segment.info.segmentId, segment.info.segmentSize);
			_storageModule->writeSegment(objectId, segment.info.segmentId,
					segment.buf, 0, segment.info.segmentSize);
		}

		// remove from map
		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
			_pendingObjectChunk.erase(objectId);
		}

		// close file and free cache
		_storageModule->closeObject(objectId);
	}

	return byteWritten;
}

void Osd::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t length) {

	_storageModule->createSegment(objectId, segmentId, length);

}

uint32_t Osd::putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t offset, uint32_t length,
		char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeSegment(objectId, segmentId, buf, offset,
			length);

	return byteWritten;
}

void Osd::recoveryProcessor(uint32_t requestId, uint32_t sockfd) {
	// TODO: recovery to be implemented
}

OsdCommunicator* Osd::getCommunicator() {
	return _osdCommunicator;
}

SegmentLocationCache* Osd::getSegmentLocationCache() {
	return _segmentLocationCache;
}

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

/**
 * Main function
 * @return 0 if success;
 */

int main(int argc, char* argv[]) {

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

	// TODO: pause before connect for now
	getchar();

	communicator->connectAllComponents();

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}
