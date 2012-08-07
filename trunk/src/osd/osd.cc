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

/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

// Global Mutex for locking _pendingObjectChunk
mutex pendingObjectChunkMutex;

Osd::Osd() {
	configLayer = new ConfigLayer("osdconfig.xml", "common.xml");
	_segmentLocationCache = new SegmentLocationCache();
	_storageModule = new StorageModule();
	_osdCommunicator = new OsdCommunicator();
	_codingModule = new CodingModule();
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
		vector<struct SegmentLocation>::const_iterator it;

		for (it = osdIdList.begin(); it != osdIdList.end(); ++it) {
			// memory of SegmentData is allocated in getSegmentRequest
			struct SegmentData segmentData;
			uint32_t osdId = (*it).osdId;
			uint32_t segmentId = (*it).segmentId;

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

	_storageModule->createObject(objectId, length);
	_osdCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

}

void Osd::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	// TODO: check integrity of object received

	_osdCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);

}

void Osd::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	// TODO: check integrity of segment received

	_osdCommunicator->replyPutSegmentEnd (requestId, sockfd, objectId, segmentId);

}

uint32_t Osd::putObjectDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeObject(objectId, buf, offset, length);

	pendingObjectChunkMutex.lock();

	// update pendingObjectChunk value
	_pendingObjectChunk[objectId] = _pendingObjectChunk[objectId] - 1;

	// close object if no more chunks
	if (_pendingObjectChunk[objectId] == 0) {
		_storageModule->closeObject(objectId);
		_pendingObjectChunk.erase(objectId);
	}

	pendingObjectChunkMutex.unlock();

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

void sendThread() {
	debug("%s", "Send Thread Start\n");
	osd->getCommunicator()->sendMessage();
	debug("%s", "Send Thread End\n");
}

/**
 * Main function
 * @return 0 if success;
 */

int main(void) {

	// create new OSD object and communicator
	osd = new Osd();

	// create new communicator
	OsdCommunicator* communicator = osd->getCommunicator();

	// start server
	const uint16_t serverPort = configLayer->getConfigInt(
			"Communication>ServerPort");
	debug("Start server on port %" PRIu16 " \n", serverPort);
	communicator->createServerSocket(serverPort);
	/*

	 // connect to MDS
	 //communicator->connectToMds();
	 */

	thread t(sendThread);
	t.detach();

	// wait for message (blocking)
	communicator->waitForMessage();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}
