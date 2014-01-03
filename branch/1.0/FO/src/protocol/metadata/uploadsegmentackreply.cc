#include <iostream>

#include "uploadsegmentackreply.hh"
#include "uploadsegmentack.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

UploadSegmentAckReplyMsg::UploadSegmentAckReplyMsg(Communicator* communicator) :
		Message(communicator) {
}

UploadSegmentAckReplyMsg::UploadSegmentAckReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, uint64_t segmentId) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_segmentId = segmentId;
}

void UploadSegmentAckReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::UploadSegmentAckReplyPro uploadSegmentAckReplyPro;

	uploadSegmentAckReplyPro.set_segmentid((long long int) _segmentId);

	if (!uploadSegmentAckReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(UPLOAD_SEGMENT_ACK_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void UploadSegmentAckReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadSegmentAckReplyPro uploadSegmentAckReplyPro;
	uploadSegmentAckReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = uploadSegmentAckReplyPro.segmentid();
	return;
}

void UploadSegmentAckReplyMsg::doHandle() {
	UploadSegmentAckMsg* uploadSegmentAckMsg = (UploadSegmentAckMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);
    uploadSegmentAckMsg->setStatus(READY);
}

void UploadSegmentAckReplyMsg::printProtocol() {
	debug("[UPLOAD_SEGMENT_ACK_REPLY] Segment ID = %" PRIu64 "\n",  _segmentId);
}
