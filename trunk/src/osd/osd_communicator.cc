/**
 * osd_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "osd_communicator.hh"
#include "../common/enums.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../common/debug.hh"
#include "segmentlocationcache.hh"

using namespace std;

/**
 * Constructor
 */

OsdCommunicator::OsdCommunicator() {

}

/**
 * Destructor
 */

OsdCommunicator::~OsdCommunicator() {

}

/**
 * Establish connection to MDS
 */

void OsdCommunicator::connectToMds() {

	// example
	string ip = "127.0.0.1";
	uint16_t port = 30000;
	ComponentType connectionType = MDS;

	// do connection
	connectAndAdd(ip, port, connectionType);
}

uint32_t OsdCommunicator::reportOsdFailure(uint32_t osdId) {
	return 0;
}

uint32_t OsdCommunicator::sendSegment(uint32_t sockfd, struct SegmentData segmentData) {
	return 0;
}

uint32_t OsdCommunicator::sendObject(uint32_t sockfd, struct ObjectData objectData) {
	return 0;
}

struct SegmentData OsdCommunicator::getSegmentRequest(uint32_t osdId, uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData;
	return segmentData;
}

list <struct SegmentLocation> OsdCommunicator::getOsdListRequest(uint64_t objectId, ComponentType dstComponent) {
	list <struct SegmentLocation> osdList;
	return osdList;
}

uint32_t OsdCommunicator::sendSegmentAck(uint64_t objectId, uint32_t segmentId,
		ComponentType dstComponent) {
	return 0;
}
