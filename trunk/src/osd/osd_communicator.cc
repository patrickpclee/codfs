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

#include "../protocol/putsegmentinitrequest.hh"
#include "../protocol/putsegmentinitreply.hh"
#include "../protocol/putsegmentendrequest.hh"
#include "../protocol/putsegmentendreply.hh"
#include "../protocol/segmentdatamsg.hh"

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

void OsdCommunicator::replyPutSegmentInit(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId, uint32_t segmentId) {

	PutSegmentInitReplyMsg* putSegmentInitReplyMsg = new PutSegmentInitReplyMsg(
			this, requestId, connectionId, objectId, segmentId);
	putSegmentInitReplyMsg->prepareProtocolMsg();

	addMessage(putSegmentInitReplyMsg);
}

void OsdCommunicator::replyPutObjectEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId) {

	PutObjectEndReplyMsg* putObjectEndReplyMsg = new PutObjectEndReplyMsg(this,
			requestId, connectionId, objectId);
	putObjectEndReplyMsg->prepareProtocolMsg();

	addMessage(putObjectEndReplyMsg);
}

void OsdCommunicator::replyPutSegmentEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId, uint32_t segmentId) {

	PutSegmentEndReplyMsg* putSegmentEndReplyMsg = new PutSegmentEndReplyMsg(
			this, requestId, connectionId, objectId, segmentId);
	putSegmentEndReplyMsg->prepareProtocolMsg();

	addMessage(putSegmentEndReplyMsg);

//TODO
}

uint32_t OsdCommunicator::reportOsdFailure(uint32_t osdId) {
	return 0;
}

uint32_t OsdCommunicator::sendSegment(uint32_t osdId, uint32_t sockfd,
		struct SegmentData segmentData) {

	uint64_t objectId = segmentData.info.objectId;
	uint32_t segmentId = segmentData.info.segmentId;
	uint32_t length = segmentData.info.segmentSize;
	char* buf = segmentData.buf;
	const uint32_t chunkCount = ((length - 1) / _chunkSize) + 1;

	// step 1: send init message, wait for ack
	debug("Put Segment Init to FD = %" PRIu32 "\n", sockfd);
	putSegmentInit(osdId, sockfd, objectId, segmentId, length, chunkCount);
	debug("Put Segment Init ACK-ed from FD = %" PRIu32 "\n", sockfd);

	// step 2: send data

	uint64_t byteToSend = 0;
	uint64_t byteProcessed = 0;
	uint64_t byteRemaining = length;

	while (byteProcessed < length) {

		if (byteRemaining > _chunkSize) {
			byteToSend = _chunkSize;
		} else {
			byteToSend = byteRemaining;
		}

		putSegmentData(osdId, sockfd, objectId, segmentId, buf, byteProcessed,
				byteToSend);
		byteProcessed += byteToSend;
		byteRemaining -= byteToSend;

	}

	// Step 3: Send End message

	putSegmentEnd(osdId, sockfd, objectId, segmentId);

	cout << "Put Segment ID = " << objectId << "." << segmentId << " Finished"
			<< endl;

	return 0;
}

uint32_t OsdCommunicator::sendObject(uint32_t sockfd,
		struct ObjectData objectData) {
	// TEST

	debug("Send object ID = %" PRIu64 " to sockfd = %" PRIu32 "\n",
			objectData.info.objectId, sockfd);

	// step 1: send init message, wait for ack

	// step 2: send data

	// step 3: send end message, wait for ack

	return 0;
}

struct SegmentData OsdCommunicator::getSegmentRequest(uint32_t osdId,
		uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData;
	return segmentData;
}

vector<struct SegmentLocation> OsdCommunicator::getOsdListRequest(
		uint64_t objectId, ComponentType dstComponent, uint32_t segmentCount) {

	vector<struct SegmentLocation> osdList;

	// TODO: request to MONITOR (HARDCODE FOR NOW)

	for (uint32_t i = 0; i < segmentCount; i++) {
		struct SegmentLocation segmentLocation;
		segmentLocation.osdId = 52001;
		segmentLocation.segmentId = 0;
		osdList.push_back(segmentLocation);
	}

	return osdList;
}

uint32_t OsdCommunicator::sendSegmentAck(uint64_t objectId, uint32_t segmentId,
		ComponentType dstComponent) {
	return 0;
}

//
// PRIVATE FUNCTIONS
//

void OsdCommunicator::putSegmentInit(uint32_t osdId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, uint32_t length,
		uint32_t chunkCount) {

	// Step 1 of the upload process

	PutSegmentInitRequestMsg* putSegmentInitRequestMsg =
			new PutSegmentInitRequestMsg(this, sockfd, objectId, segmentId,
					length, chunkCount);

	putSegmentInitRequestMsg->prepareProtocolMsg();
	addMessage(putSegmentInitRequestMsg, true);

	MessageStatus status = putSegmentInitRequestMsg->waitForStatusChange();
	if (status == READY) {
		waitAndDelete(putSegmentInitRequestMsg);
		return;
	} else {
		debug("%s\n", "Put Segment Init Failed");
		exit(-1);
	}

}

void OsdCommunicator::putSegmentData(uint32_t osdId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId, char* buf, uint64_t offset,
		uint32_t length) {

	// Step 2 of the upload process
	SegmentDataMsg* segmentDataMsg = new SegmentDataMsg(this, sockfd, objectId,
			segmentId, offset, length);

	segmentDataMsg->prepareProtocolMsg();
	segmentDataMsg->preparePayload(buf + offset, length);

	addMessage(segmentDataMsg, false);
}

void OsdCommunicator::putSegmentEnd(uint32_t osdId, uint32_t sockfd,
		uint64_t objectId, uint32_t segmentId) {

	// Step 3 of the upload process

	PutSegmentEndRequestMsg* putSegmentEndRequestMsg =
			new PutSegmentEndRequestMsg(this, sockfd, objectId, segmentId);

	putSegmentEndRequestMsg->prepareProtocolMsg();
	addMessage(putSegmentEndRequestMsg, true);

	MessageStatus status = putSegmentEndRequestMsg->waitForStatusChange();
	if (status == READY) {
		waitAndDelete(putSegmentEndRequestMsg);
		return;
	} else {
		debug("%s\n", "Put Segment Init Failed");
		exit(-1);
	}
}
