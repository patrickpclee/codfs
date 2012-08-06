/**
 * osd_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "osd_communicator.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/putobjectinitreply.hh"
#include "../protocol/putobjectendreply.hh"
#include "../common/debug.hh"
#include "segmentlocationcache.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"

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

void OsdCommunicator::replyPutObjectInit(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId) {

	PutObjectInitReplyMsg* putObjectInitReplyMsg = new PutObjectInitReplyMsg(
			this, requestId, connectionId, objectId);
	putObjectInitReplyMsg->prepareProtocolMsg();

	addMessage(putObjectInitReplyMsg);
}

void OsdCommunicator::replyPutObjectEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId) {

	PutObjectEndReplyMsg* putObjectEndReplyMsg = new PutObjectEndReplyMsg(this,
			requestId, connectionId, objectId);
	putObjectEndReplyMsg->prepareProtocolMsg();

	addMessage(putObjectEndReplyMsg);
}

uint32_t OsdCommunicator::reportOsdFailure(uint32_t osdId) {
	return 0;
}

uint32_t OsdCommunicator::sendSegment(uint32_t sockfd,
		struct SegmentData segmentData) {

	// step 1: send init message, wait for ack

	// step 2: send data

	// step 3: send end message, wait for ack

	// step 4: free memory
	MemoryPool::getInstance().poolFree(segmentData.buf);

	return 0;
}

uint32_t OsdCommunicator::sendObject(uint32_t sockfd,
		struct ObjectData objectData) {
	// TEST

	debug("Send object ID = %lu to sockfd = %d, content = %s\n",
			objectData.info.objectId, sockfd, objectData.buf);

	// step 1: send init message, wait for ack

	// step 2: send data

	// step 3: send end message, wait for ack

	// step 4: free memory
	MemoryPool::getInstance().poolFree(objectData.buf);

	return 0;
}

struct SegmentData OsdCommunicator::getSegmentRequest(uint32_t osdId,
		uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData;
	return segmentData;
}

vector<struct SegmentLocation> OsdCommunicator::getOsdListRequest(
		uint64_t objectId, ComponentType dstComponent) {
	vector<struct SegmentLocation> osdList;
	return osdList;
}

uint32_t OsdCommunicator::sendSegmentAck(uint64_t objectId, uint32_t segmentId,
		ComponentType dstComponent) {
	return 0;
}
