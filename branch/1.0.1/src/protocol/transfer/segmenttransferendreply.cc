#include "segmenttransferendreply.hh"
#include "segmenttransferendrequest.hh"
#include "putsmallsegmentrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

SegmentTransferEndReplyMsg::SegmentTransferEndReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

SegmentTransferEndReplyMsg::SegmentTransferEndReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t segmentId, bool isSmallSegment) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;
	_segmentId = segmentId;
	_isSmallSegment = isSmallSegment;
}

void SegmentTransferEndReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentTransferEndReplyPro segmentTransferEndReplyPro;
	segmentTransferEndReplyPro.set_segmentid(_segmentId);
	segmentTransferEndReplyPro.set_issmallsegment(_isSmallSegment);

	if (!segmentTransferEndReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (SEGMENT_TRANSFER_END_REPLY);
	setProtocolMsg(serializedString);

}

void SegmentTransferEndReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SegmentTransferEndReplyPro segmentTransferEndReplyPro;
	segmentTransferEndReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = segmentTransferEndReplyPro.segmentid();
	_isSmallSegment = segmentTransferEndReplyPro.issmallsegment();

}

void SegmentTransferEndReplyMsg::doHandle() {
    if (_isSmallSegment) {
        PutSmallSegmentRequestMsg* putSmallSegmentRequestMsg =
                (PutSmallSegmentRequestMsg*) _communicator->popWaitReplyMessage(
                        _msgHeader.requestId);
        putSmallSegmentRequestMsg->setStatus(READY);
    } else {
        SegmentTransferEndRequestMsg* putSegmentEndRequestMsg =
                (SegmentTransferEndRequestMsg*) _communicator->popWaitReplyMessage(
                        _msgHeader.requestId);
        putSegmentEndRequestMsg->setStatus(READY);
    }
}

void SegmentTransferEndReplyMsg::printProtocol() {
	debug("[SEGMENT_TRANSFER_END_REPLY] Segment ID = %" PRIu64 "\n", _segmentId);
}
