/**
 * osd_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "osd.hh"
#include "osd_communicator.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"
#include "../common/metadata.hh"
#include "../protocol/metadata/uploadobjectack.hh"
#include "../protocol/metadata/listdirectoryrequest.hh"
#include "../protocol/metadata/getobjectinforequest.hh"
#include "../protocol/transfer/putobjectinitreply.hh"
#include "../protocol/transfer/getsegmentinitrequest.hh"
#include "../protocol/transfer/putsegmentinitrequest.hh"
#include "../protocol/transfer/putsegmentinitreply.hh"
#include "../protocol/transfer/objecttransferendreply.hh"
#include "../protocol/transfer/segmenttransferendrequest.hh"
#include "../protocol/transfer/segmenttransferendreply.hh"
#include "../protocol/transfer/segmentdatamsg.hh"
#include "../protocol/nodelist/getsecondarylistrequest.hh"
#include "../protocol/status/osdstartupmsg.hh"

using namespace std;

extern Osd* osd;

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

	ObjectTransferEndReplyMsg* putObjectEndReplyMsg =
			new ObjectTransferEndReplyMsg(this, requestId, connectionId,
					objectId);
	putObjectEndReplyMsg->prepareProtocolMsg();

	addMessage(putObjectEndReplyMsg);
}

void OsdCommunicator::replyPutSegmentEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId, uint32_t segmentId) {

	SegmentTransferEndReplyMsg* segmentTransferEndReplyMsg =
			new SegmentTransferEndReplyMsg(this, requestId, connectionId,
					objectId, segmentId);
	segmentTransferEndReplyMsg->prepareProtocolMsg();

	addMessage(segmentTransferEndReplyMsg);

//TODO
}

uint32_t OsdCommunicator::reportOsdFailure(uint32_t osdId) {
	return 0;
}

uint32_t OsdCommunicator::sendSegment(uint32_t sockfd,
		struct SegmentData segmentData) {

	uint64_t objectId = segmentData.info.objectId;
	uint32_t segmentId = segmentData.info.segmentId;
	uint32_t length = segmentData.info.segmentSize;
	char* buf = segmentData.buf;
	const uint32_t chunkCount = ((length - 1) / _chunkSize) + 1;

	// step 1: send init message, wait for ack
	debug("Put Segment Init to FD = %" PRIu32 "\n", sockfd);
	putSegmentInit(sockfd, objectId, segmentId, length, chunkCount);
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

		putSegmentData(sockfd, objectId, segmentId, buf, byteProcessed,
				byteToSend);
		byteProcessed += byteToSend;
		byteRemaining -= byteToSend;

	}

	// Step 3: Send End message

	putSegmentEnd(sockfd, objectId, segmentId);

	cout << "Put Segment ID = " << objectId << "." << segmentId << " Finished"
			<< endl;

	return 0;
}

void OsdCommunicator::getSegmentRequest(uint32_t osdId, uint64_t objectId,
		uint32_t segmentId) {

	struct SegmentData segmentData;

	uint32_t dstSockfd = getSockfdFromId(osdId);
	GetSegmentInitRequestMsg* getSegmentInitRequestMsg =
			new GetSegmentInitRequestMsg(this, dstSockfd, objectId, segmentId);
	getSegmentInitRequestMsg->prepareProtocolMsg();

	addMessage(getSegmentInitRequestMsg, false);

}

vector<struct SegmentLocation> OsdCommunicator::getOsdListRequest(
		uint64_t objectId, ComponentType dstComponent, uint32_t segmentCount) {

	GetSecondaryListRequestMsg* getSecondaryListRequestMsg =
			new GetSecondaryListRequestMsg(this, getMonitorSockfd(), segmentCount);
	getSecondaryListRequestMsg->prepareProtocolMsg();

	addMessage(getSecondaryListRequestMsg, true);
	MessageStatus status = getSecondaryListRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<struct SegmentLocation> osdList = 
				getSecondaryListRequestMsg->getSecondaryList();
		return osdList;
	}

	return {};
	/* 
	srand(time(NULL));

	// TODO: request to MONITOR (HARDCODE FOR NOW)

	for (uint32_t i = 0; i < segmentCount; i++) {
		struct SegmentLocation segmentLocation;

		// DEBUG 1: random assignment
		segmentLocation.osdId = rand() % 2 + 52000;

		// DEBUG 2: must be local
		//segmentLocation.osdId = _componentId;

		// DEBUG 3: must be foreign
		if (_componentId == 52000) {
			segmentLocation.osdId = 52001;
		} else {
			segmentLocation.osdId = 52000;
		}

		segmentLocation.segmentId = 0;
		osdList.push_back(segmentLocation);
	}

	return osdList;
	*/

}

uint32_t OsdCommunicator::sendSegmentAck(uint64_t objectId, uint32_t segmentId,
		ComponentType dstComponent) {
	return 0;
}

//
// PRIVATE FUNCTIONS
//

void OsdCommunicator::putSegmentInit(uint32_t sockfd, uint64_t objectId,
		uint32_t segmentId, uint32_t length, uint32_t chunkCount) {

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

void OsdCommunicator::putSegmentData(uint32_t sockfd, uint64_t objectId,
		uint32_t segmentId, char* buf, uint64_t offset, uint32_t length) {

	// Step 2 of the upload process
	SegmentDataMsg* segmentDataMsg = new SegmentDataMsg(this, sockfd, objectId,
			segmentId, offset, length);

	segmentDataMsg->prepareProtocolMsg();
	segmentDataMsg->preparePayload(buf + offset, length);

	addMessage(segmentDataMsg, false);
}

void OsdCommunicator::putSegmentEnd(uint32_t sockfd, uint64_t objectId,
		uint32_t segmentId) {

	// Step 3 of the upload process

	SegmentTransferEndRequestMsg* segmentTransferEndRequestMsg =
			new SegmentTransferEndRequestMsg(this, sockfd, objectId, segmentId);

	segmentTransferEndRequestMsg->prepareProtocolMsg();
	addMessage(segmentTransferEndRequestMsg, true);

	MessageStatus status = segmentTransferEndRequestMsg->waitForStatusChange();
	if (status == READY) {
		waitAndDelete(segmentTransferEndRequestMsg);
		return;
	} else {
		debug("%s\n", "Put Segment Init Failed");
		exit(-1);
	}
}

void OsdCommunicator::objectUploadAck(uint64_t objectId,
		CodingScheme codingScheme, string codingSetting,
		vector<uint32_t> nodeList, string checksum) {
	uint32_t mdsSockFd = getMdsSockfd();

	UploadObjectAckMsg* uploadObjectAckMsg = new UploadObjectAckMsg(this,
			mdsSockFd, objectId, codingScheme, codingSetting, nodeList, checksum);

	uploadObjectAckMsg->prepareProtocolMsg();
	addMessage(uploadObjectAckMsg, false);

	/*
	 addMessage(objectUploadAckRequestMsg, true);

	 MessageStatus status = objectUploadAckRequestMsg->waitForStatusChange();
	 if(status == READY) {
	 waitAndDelete(objectUploadAckMsg);
	 return ;
	 } else {
	 debug("Object Upload Ack Failed [%" PRIu64 "]\n", objectId);
	 exit(-1);
	 }
	 */
}

// DOWNLOAD

struct ObjectTransferOsdInfo OsdCommunicator::getObjectInfoRequest(
		uint64_t objectId) {

	struct ObjectTransferOsdInfo objectInfo = { };
	uint32_t mdsSockFd = getMdsSockfd();

	GetObjectInfoRequestMsg* getObjectInfoRequestMsg =
			new GetObjectInfoRequestMsg(this, mdsSockFd, objectId);
	getObjectInfoRequestMsg->prepareProtocolMsg();
	addMessage(getObjectInfoRequestMsg, true);

	MessageStatus status = getObjectInfoRequestMsg->waitForStatusChange();
	if (status == READY) {
		objectInfo._id = objectId;
		objectInfo._size = getObjectInfoRequestMsg->getObjectSize();
		objectInfo._codingScheme = getObjectInfoRequestMsg->getCodingScheme();
		objectInfo._codingSetting = getObjectInfoRequestMsg->getCodingSetting();
		objectInfo._checksum = getObjectInfoRequestMsg->getChecksum();
		objectInfo._osdList = getObjectInfoRequestMsg->getNodeList();
		waitAndDelete(getObjectInfoRequestMsg);
	} else {
		debug("%s\n", "Get Object Info Request Failed");
		exit(-1);
	}

	return objectInfo;
}

void OsdCommunicator::registerToMonitor() {
	OsdStartupMsg* startupMsg = new OsdStartupMsg(this, getMonitorSockfd(), 
		osd->getOsdId(), osd->getFreespace(), osd->getCpuLoadavg(0));
	startupMsg->prepareProtocolMsg();
	addMessage(startupMsg);
}
