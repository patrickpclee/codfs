/**
 * osd.cc
 */

#include <stdio.h>
#include "osd.hh"

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

list<uint32_t> secOsdListHandler(uint64_t objectId) {

}

ObjectData getObjectHandler(uint64_t objectId) {

}

SegmentData getSegmentHandler(uint64_t objectId, uint32_t segmentId) {

}

uint32_t objectTrunkHandler(uint64_t objectId, uint32_t offset, uint32_t length,
		char* buf) {

}

uint32_t segmentTrunkHandler(uint64_t objectId, uint32_t segmentId,
		uint32_t offset, uint32_t length, char* buf) {

}

uint32_t recoveryHandler(uint64_t objectId) {

}

list<SegmentData> encodeObjectToSegment(ObjectData objectData) {

}

ObjectData decodeSegmentToObject(uint64_t objectId,
		list<SegmentData> segmentData) {

}

uint32_t sendAckToMds(uint64_t objectId, uint32_t segmentId) {

}

uint32_t sendAckToClient(uint32_t fileId) {

}

uint32_t getSegmentRequest(uint64_t objectId, uint32_t segmentId) {

}

uint32_t getSecOsdListRequest(uint64_t objectId) {

}

SegmentData getSegmentFromStroage(uint64_t objectId, uint32_t segmentId) {

}

uint32_t sendSegmentToOsd(SegmentData segmentData) {

}

uint32_t sendObjectToClient(ObjectData objectData) {

}

uint32_t saveSegmentToStorage(SegmentData segmentData) {

}

uint32_t degradedRead(uint64_t objectId) {

}

uint32_t reportOsdFailure(uint32_t osdId) {

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
