/**
 * osd.cc
 */

#include <stdio.h>
#include <vector>
#include "osd.hh"
#include "../config/config.hh"

ConfigLayer* configLayer;

Osd::Osd() {
	cout << "OSD Created" << endl;
	_osdCommunicator = new OsdCommunicator();
}

Osd::~Osd() {
	delete _osdCommunicator;
}

/**
 * Save the received OSD list into the SegmentLocationCache
 * 1. If existing list is found, delete it
 * 2. Insert the new list into the cache
 */

uint32_t Osd::osdListProcessor(uint32_t sockfd, uint64_t objectId,
		list<uint32_t> osdList) {

	/*

	_segmentLocationCache->deleteSegmentLocation(objectId);
	_segmentLocationCache->writeSegmentLocation(objectId, osdList);
	*/

	return 0;
}

/**
 * Send the object to the target
 */

uint32_t Osd::getObjectProcessor(uint32_t sockfd, uint64_t objectId) {
	list<uint32_t> osdIdList;

	/*
	// check if I have the object
	if (_storageModule) {

	}

	try {
		osdIdList = getSegmentLocationCache()->readSegmentLocation(objectId);
	} catch (CacheMissException &e) {

	}
	*/

	return 0;

}

OsdCommunicator* Osd::getOsdCommunicator() {
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

	// create new OSD object
	Osd* osd = new Osd();

	// new ConfigLayer object
	configLayer = new ConfigLayer();

	// create new communicator
	OsdCommunicator* communicator = osd->getOsdCommunicator();

	// connect to MDS
	communicator->connectToMds();

	// wait for message
	communicator->waitForMessage();

	// cleanup
	delete osd;
	delete configLayer;

	return 0;
}
