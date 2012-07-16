/**
 * osd.cc
 */

#include <stdio.h>
#include <vector>
#include "osd.hh"

/*
 * GLOBAL VARIABLES
 */

/**
 * OSD Constructor
 * Initialise a communicator
 */

Osd::Osd() {
	cout << "OSD Created" << endl;
	_osdCommunicator = new OsdCommunicator();
}

/**
 * OSD Destructor
 */
Osd::~Osd() {
	delete _osdCommunicator;
}

/**
 * Get the OSD Communicator
 * @return Pointer to OSD Communicator
 */

OsdCommunicator* Osd::getOsdCommunicator() {
	return _osdCommunicator;
}

/**
 * Get the Cache
 * @return Point to Cache
 */

SegmentLocationCache* Osd::getCache() {
	return _segmentLocationCache;
}

list<uint32_t> Osd::secOsdListHandler(uint64_t objectId, list<uint32_t> osdList) {
	// TODO: Save to cache

}

ObjectData Osd::getObjectHandler(uint64_t objectId) {
	list <uint32_t> osdIdList;
	try {
		osdIdList = getCache()->readSegmentLocation (objectId);
	} catch (CacheMissException &e) {
		// Cache Miss
		getSecOsdListRequest(objectId);
		// TODO: how to continue logic after async-request?
	}
}

/**
 * Handle request for segment from other OSDs
 * @param objectId Object ID which the segment belongs to
 * @param segmentId Segment ID
 * @return SegmentData structure
 */

SegmentData Osd::getSegmentHandler(uint64_t objectId, uint32_t segmentId) {
	//string filepath = getSegmentPath (objectId, segmentId);



}

uint32_t Osd::objectTrunkHandler(uint64_t objectId, uint32_t offset, uint32_t length,
		vector<unsigned char> buf) {

}

uint32_t Osd::segmentTrunkHandler(uint64_t objectId, uint32_t segmentId,
		uint32_t offset, uint32_t length, vector<unsigned char> buf) {

}

uint32_t Osd::recoveryHandler(uint64_t objectId) {

}

list<SegmentData> Osd::encodeObjectToSegment(ObjectData objectData) {

}

ObjectData Osd::decodeSegmentToObject(uint64_t objectId,
		list<SegmentData> segmentData) {

}

uint32_t Osd::sendAckToMds(uint64_t objectId, uint32_t segmentId) {

}

uint32_t Osd::sendAckToClient(uint32_t fileId) {

}

uint32_t Osd::getSegmentRequest(uint64_t objectId, uint32_t segmentId) {

}

uint32_t Osd::getSecOsdListRequest(uint64_t objectId) {

}

SegmentData Osd::getSegmentFromStroage(uint64_t objectId, uint32_t segmentId) {

}

uint32_t Osd::sendSegmentToOsd(SegmentData segmentData) {

}

uint32_t Osd::sendObjectToClient(ObjectData objectData) {

}

uint32_t Osd::saveSegmentToStorage(SegmentData segmentData) {

}

uint32_t Osd::degradedRead(uint64_t objectId) {

}

uint32_t Osd::reportOsdFailure(uint32_t osdId) {

}

/**
 * Main function
 * @return 0 if success;
 */

int main(void) {

	// create new OSD object
	Osd* osd = new Osd();

	// create new communicator
	OsdCommunicator* communicator = osd->getOsdCommunicator();

	// connect to MDS
	communicator->connectToMds();

	// TEST: list directory from MDS
//	communicator->listDirectoryRequest(1, "/");

// cleanup
	delete osd;
	cout << "OSD Destroyed" << endl;

	return 0;
}
