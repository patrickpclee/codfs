#include <iostream>

#include "saveobjectlistreply.hh"
#include "saveobjectlistrequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/debug.hh"

SaveObjectListReplyMsg::SaveObjectListReplyMsg(Communicator* communicator) :
	Message(communicator) {
}

SaveObjectListReplyMsg::SaveObjectListReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId) :
	Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_fileId = fileId;
}

void SaveObjectListReplyMsg::prepareProtocolMsg()
{
	string serializedString;

	ncvfs::SaveObjectListReplyPro saveObjectListReplyPro;

	saveObjectListReplyPro.set_fileid(_fileId);

	if (!saveObjectListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SAVE_OBJECT_LIST_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void SaveObjectListReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SaveObjectListReplyPro saveObjectListReplyPro;
	saveObjectListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = saveObjectListReplyPro.fileid();
	
	return ;
}

void SaveObjectListReplyMsg::doHandle() {
	SaveObjectListRequestMsg* saveObjectListRequestMsg = (SaveObjectListRequestMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);

	saveObjectListRequestMsg->setStatus(READY);
}

void SaveObjectListReplyMsg::printProtocol() {
	debug("[SAVE_OBJECT_LIST_REPLY] File ID = %" PRIu32 "\n", _fileId);
}
