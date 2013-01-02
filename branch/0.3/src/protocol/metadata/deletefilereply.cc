#include <iostream>

#include "deletefilereply.hh"
#include "deletefilerequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

DeleteFileReplyMsg::DeleteFileReplyMsg(Communicator* communicator) :
	Message(communicator) {
}

DeleteFileReplyMsg::DeleteFileReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId) :
	Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_fileId = fileId;
	
}

void DeleteFileReplyMsg::prepareProtocolMsg()
{
	string serializedString;

	ncvfs::DeleteFileReplyPro deleteFileReplyPro;

	deleteFileReplyPro.set_fileid(_fileId);

	if (!deleteFileReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(DELETE_FILE_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void DeleteFileReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::DeleteFileReplyPro deleteFileReplyPro;
	deleteFileReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = deleteFileReplyPro.fileid();
	
	return ;
}

void DeleteFileReplyMsg::doHandle() {
	DeleteFileRequestMsg* deleteFileRequestMsg = (DeleteFileRequestMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);
	deleteFileRequestMsg->setStatus(READY);
}

void DeleteFileReplyMsg::printProtocol() {
	debug("[DELETE_FILE_REPLY] File ID = %" PRIu32 "\n", _fileId);
}
