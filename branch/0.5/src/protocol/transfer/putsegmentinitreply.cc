#include "putsegmentinitreply.hh"
#include "putsegmentinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

PutSegmentInitReplyMsg::PutSegmentInitReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitReplyMsg::PutSegmentInitReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t segmentId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_segmentId = segmentId;
	
}

void PutSegmentInitReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitReplyPro putSegmentInitReplyPro;
	putSegmentInitReplyPro.set_segmentid(_segmentId);

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

	_segmentId = putSegmentInitReplyPro.segmentid();

}

void PutSegmentInitReplyMsg::doHandle() {
	PutSegmentInitRequestMsg* putSegmentInitRequestMsg =
			(PutSegmentInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putSegmentInitRequestMsg->setStatus(READY);
}

void PutSegmentInitReplyMsg::printProtocol() {
	debug("[PUT_SEGMENT_INIT_REPLY] Segment ID = %" PRIu64 "\n", _segmentId);
}
