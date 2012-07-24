/**
 * osd.cc
 */

#include <stdio.h>
#include <vector>
#include "osd.hh"
#include "segmentlocationcache.hh"
#include "../common/debug.hh"
#include "../config/config.hh"

ConfigLayer* configLayer;

Osd::Osd() {
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

uint32_t Osd::osdListProcessor(uint32_t sockfd, uint64_t objectId,
		list<SegmentLocation> osdList) {

	_segmentLocationCache->deleteSegmentLocation(objectId);
	_segmentLocationCache->writeSegmentLocation(objectId, osdList);

	return 0;
}

/**
 * Send the object to the target
 */

void Osd::getObjectProcessor(uint32_t sockfd, uint64_t objectId) {

	list<struct SegmentData> segmentDataList;
	list<struct SegmentLocation> osdIdList;
	struct ObjectData objectData;

	if (_storageModule->isObjectExist(objectId)) {
		// if object exists in cache
		objectData = _storageModule->readObject(objectId);
	} else {
		// get osdIDList from cache, if failed update it from MDS
		try {
			osdIdList = _segmentLocationCache->readSegmentLocation(objectId);
		} catch (CacheMissException &e) {
			osdIdList = _osdCommunicator->getOsdListRequest(objectId, MDS);
			_segmentLocationCache->writeSegmentLocation(objectId, osdIdList);
		}

		// get segments from the OSD one by one
		list<struct SegmentLocation>::const_iterator it;

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
	}

	// memory of objectData is allocated in decodeSegmentToObject
	// TODO: decodeSegmentToObject should free memory in segmentDataList
	objectData = _codingModule->decodeSegmentToObject(objectId,
			segmentDataList);

	_osdCommunicator->sendObject(sockfd, objectData);

	return;
}

void Osd::getSegmentProcessor(uint32_t sockfd, uint64_t objectId,
		uint32_t segmentId) {

	struct SegmentData segmentData;
	segmentData = _storageModule->readSegment(objectId, segmentId);
	_osdCommunicator->sendSegment(sockfd, segmentData);

	return;
}

OsdCommunicator* Osd::getCommunicator() {
	return _osdCommunicator;
}

SegmentLocationCache* Osd::getSegmentLocationCache() {
	return _segmentLocationCache;
}

/**
 * Main function
 * @return 0 if success;
 */

int main(void) {

	// create new OSD object and communicator
	Osd* osd = new Osd();

	// new ConfigLayer object (global)
	configLayer = new ConfigLayer("osdconfig.xml");

	// create new communicator
	OsdCommunicator* communicator = osd->getCommunicator();

	// start server
	const uint16_t serverPort = configLayer->getConfigInt(
			"Communication>ServerPort");
	debug("Start server on port %d\n", serverPort);
	communicator->createServerSocket(serverPort);

	// connect to MDS
	//communicator->connectToMds();

	// wait for message (blocking)
	communicator->waitForMessage();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}
