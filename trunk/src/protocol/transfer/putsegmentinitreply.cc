#include "putsegmentinitreply.hh"
#include "putsegmentinitrequest.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"

PutSegmentInitReplyMsg::PutSegmentInitReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitReplyMsg::PutSegmentInitReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void PutSegmentInitReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitReplyPro putSegmentInitReplyPro;
	putSegmentInitReplyPro.set_objectid(_objectId);

	if (!putSegmentInitReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_SEGMENT_INIT_REPLY);
	setProtocolMsg(serializedString);

}

void PutSegmentInitReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentInitReplyPro putSegmentInitReplyPro;
	putSegmentInitReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putSegmentInitReplyPro.objectid();
	_segmentId = putSegmentInitReplyPro.segmentid();

}

void PutSegmentInitReplyMsg::doHandle() {

	PutSegmentInitRequestMsg* putSegmentInitRequestMsg =
			(PutSegmentInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putSegmentInitRequestMsg->setStatus(READY);
}

void PutSegmentInitReplyMsg::printProtocol() {
	debug("[PUT_SEGMENT_INIT_REPLY] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 "\n", _objectId, _segmentId);
}
