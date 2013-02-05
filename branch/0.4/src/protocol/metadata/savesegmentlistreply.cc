#include <iostream>

#include "savesegmentlistreply.hh"
#include "savesegmentlistrequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/debug.hh"

SaveSegmentListReplyMsg::SaveSegmentListReplyMsg(Communicator* communicator) :
	Message(communicator) {
}

SaveSegmentListReplyMsg::SaveSegmentListReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId) :
	Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_fileId = fileId;
}

void SaveSegmentListReplyMsg::prepareProtocolMsg()
{
	string serializedString;

	ncvfs::SaveSegmentListReplyPro saveSegmentListReplyPro;

	saveSegmentListReplyPro.set_fileid(_fileId);

	if (!saveSegmentListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SAVE_SEGMENT_LIST_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void SaveSegmentListReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SaveSegmentListReplyPro saveSegmentListReplyPro;
	saveSegmentListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = saveSegmentListReplyPro.fileid();
	
	return ;
}

void SaveSegmentListReplyMsg::doHandle() {
	SaveSegmentListRequestMsg* saveSegmentListRequestMsg = (SaveSegmentListRequestMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);

	saveSegmentListRequestMsg->setStatus(READY);
}

void SaveSegmentListReplyMsg::printProtocol() {
	debug("[SAVE_SEGMENT_LIST_REPLY] File ID = %" PRIu32 "\n", _fileId);
}
