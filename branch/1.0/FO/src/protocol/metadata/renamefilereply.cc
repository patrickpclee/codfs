#include <iostream>

#include "renamefilereply.hh"
#include "renamefilerequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/debug.hh"

RenameFileReplyMsg::RenameFileReplyMsg(Communicator* communicator) :
	Message(communicator) {
}

RenameFileReplyMsg::RenameFileReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId) :
	Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_fileId = fileId;
}

void RenameFileReplyMsg::prepareProtocolMsg()
{
	string serializedString;

	ncvfs::RenameFileReplyPro renameFileReplyPro;

	renameFileReplyPro.set_fileid(_fileId);

	if (!renameFileReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(RENAME_FILE_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void RenameFileReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RenameFileReplyPro renameFileReplyPro;
	renameFileReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = renameFileReplyPro.fileid();
	
	return ;
}

void RenameFileReplyMsg::doHandle() {
	RenameFileRequestMsg* renameFileRequestMsg = (RenameFileRequestMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);

	renameFileRequestMsg->setStatus(READY);
}

void RenameFileReplyMsg::printProtocol() {
	debug("[RENAME_FILE_REPLY] File ID = %" PRIu32 "\n", _fileId);
}
