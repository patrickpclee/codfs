#include <iostream>

#include "uploadfilereply.hh"
#include "uploadfilerequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

UploadFileReplyMsg::UploadFileReplyMsg(Communicator* communicator) :
	Message(communicator) {
}

UploadFileReplyMsg::UploadFileReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId, vector<uint64_t> objectList, vector<uint32_t> primaryList) :
	Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_fileId = fileId;
	_objectList = objectList;
	_primaryList = primaryList;
	_threadPoolLevel = 1;
}

void UploadFileReplyMsg::prepareProtocolMsg()
{
	string serializedString;

	ncvfs::UploadFileReplyPro uploadFileReplyPro;

	vector<uint64_t>::iterator it;

	for (it = _objectList.begin(); it < _objectList.end(); ++it) {
		uploadFileReplyPro.add_objectlist(*it);
	}

	vector<uint32_t>::iterator it2;

	for (it2 = _primaryList.begin(); it2 < _primaryList.end(); ++it2) {
		uploadFileReplyPro.add_primarylist(*it2);
		debug("%d\n",*it2);
	}

	uploadFileReplyPro.set_fileid(_fileId);

	if (!uploadFileReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(UPLOAD_FILE_REPLY);
	setProtocolMsg(serializedString);

	return;
}

void UploadFileReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadFileReplyPro uploadFileReplyPro;
	uploadFileReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = uploadFileReplyPro.fileid();
	
	for(int i = 0; i < uploadFileReplyPro.objectlist_size(); ++i) {
		_objectList.push_back(uploadFileReplyPro.objectlist(i));
		_primaryList.push_back(uploadFileReplyPro.primarylist(i));
	}

	return ;
}

void UploadFileReplyMsg::doHandle() {
	UploadFileRequestMsg* uploadFileRequestMsg = (UploadFileRequestMsg*) _communicator->popWaitReplyMessage(_msgHeader.requestId);

	uploadFileRequestMsg->setFileId(_fileId);
	uploadFileRequestMsg->setObjectList(_objectList);
	uploadFileRequestMsg->setPrimaryList(_primaryList);
	uploadFileRequestMsg->setStatus(READY);
}

void UploadFileReplyMsg::printProtocol() {
	debug("[UPLOAD_FILE_REPLY] File ID = %" PRIu32 "\n", _fileId);
}
