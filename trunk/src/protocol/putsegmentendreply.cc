#include "putsegmentendreply.hh"
#include "putsegmentendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

PutSegmentEndReplyMsg::PutSegmentEndReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentEndReplyMsg::PutSegmentEndReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t objectId, uint32_t segmentId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void PutSegmentEndReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentEndReplyPro putSegmentEndReplyPro;
	putSegmentEndReplyPro.set_objectid(_objectId);
	putSegmentEndReplyPro.set_segmentid(_segmentId);

	if (!putSegmentEndReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_SEGMENT_END_REPLY);
	setProtocolMsg(serializedString);

}

void PutSegmentEndReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentEndReplyPro putSegmentEndReplyPro;
	putSegmentEndReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putSegmentEndReplyPro.objectid();
	_segmentId = putSegmentEndReplyPro.segmentid();

}

void PutSegmentEndReplyMsg::doHandle() {
	PutSegmentEndRequestMsg* putSegmentEndRequestMsg =
			(PutSegmentEndRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putSegmentEndRequestMsg->setStatus(READY);
}

void PutSegmentEndReplyMsg::printProtocol() {
	debug("[PUT_SEGMENT_END_REPLY] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 "\n", _objectId, _segmentId);
}
