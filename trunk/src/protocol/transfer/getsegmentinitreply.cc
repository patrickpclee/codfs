#include <iostream>
#include "getsegmentinitrequest.hh"
#include "getsegmentinitreply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

GetSegmentInitReplyMsg::GetSegmentInitReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentInitReplyMsg::GetSegmentInitReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, uint64_t objectId,
		uint32_t segmentId, uint32_t segmentSize, uint32_t chunkCount) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_objectId = objectId;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_chunkCount = chunkCount;
}

void GetSegmentInitReplyMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::GetSegmentInitReplyPro getSegmentInitReplyPro;
	getSegmentInitReplyPro.set_objectid(_objectId);
	getSegmentInitReplyPro.set_segmentid(_segmentId);
	getSegmentInitReplyPro.set_segmentsize(_segmentSize);
	getSegmentInitReplyPro.set_chunkcount(_chunkCount);

	if (!getSegmentInitReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_SEGMENT_INIT_REPLY);
	setProtocolMsg(serializedString);

}

void GetSegmentInitReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentInitReplyPro getSegmentInitReplyPro;
	getSegmentInitReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getSegmentInitReplyPro.objectid();
	_segmentId = getSegmentInitReplyPro.segmentid();
	_segmentSize = getSegmentInitReplyPro.segmentsize();
	_chunkCount = getSegmentInitReplyPro.chunkcount();
}

void GetSegmentInitReplyMsg::doHandle() {
	GetSegmentInitRequestMsg* getSegmentInitRequestMsg =
			(GetSegmentInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getSegmentInitRequestMsg->setSegmentSize(_segmentSize);
	getSegmentInitRequestMsg->setChunkCount(_chunkCount);
	getSegmentInitRequestMsg->setStatus(READY);
}

void GetSegmentInitReplyMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_INIT] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 ", Segment Size = %" PRIu32 ", ChunkCount = %" PRIu32 "\n",
			_objectId, _segmentId, _segmentSize, _chunkCount);
}
