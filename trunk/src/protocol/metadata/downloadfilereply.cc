/**
 * downloadfilereply.cc
 */

#include <iostream>
#include "downloadfilerequest.hh"
#include "downloadfilereply.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"
#include "../../mds/mds.hh"

#ifdef COMPILE_FOR_MDS
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

DownloadFileReplyMsg::DownloadFileReplyMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
DownloadFileReplyMsg::DownloadFileReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, uint32_t clientId, uint32_t fileId,
		vector<uint64_t> objectList, vector<uint32_t> primaryList) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_clientId = clientId;
	_fileId = fileId;
	_objectList = objectList;
	_primaryList = primaryList;
}

void DownloadFileReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DownloadFileReplyPro downloadFileReplyPro;

	downloadFileReplyPro.set_clientid(_clientId);
	downloadFileReplyPro.set_fileid(_fileId);

	for (auto objectID : _objectList) {
		downloadFileReplyPro.add_objectlist(objectID);
	}

	for (auto primaryID : _primaryList) {
		downloadFileReplyPro.add_primarylist(primaryID);
	}

	if (!downloadFileReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (DOWNLOAD_FILE_REPLY);
	setProtocolMsg(serializedString);

}

void DownloadFileReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::DownloadFileReplyPro downloadFileReplyPro;
	downloadFileReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = downloadFileReplyPro.clientid();
	_fileId = downloadFileReplyPro.fileid();

	for (int i = 0; i < downloadFileReplyPro.objectlist_size(); i++) {
		_objectList.push_back(downloadFileReplyPro.objectlist(i));
	}

	for (int i = 0; i < downloadFileReplyPro.primarylist_size(); i++) {
		_primaryList.push_back(downloadFileReplyPro.primarylist(i));
	}

}

void DownloadFileReplyMsg::doHandle() {
	DownloadFileRequestMsg* downloadFileRequestMsg =
			(DownloadFileRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	downloadFileRequestMsg->setObjectList(_objectList);
	downloadFileRequestMsg->setPrimaryList(_primaryList);
	downloadFileRequestMsg->setStatus(READY);
}

void DownloadFileReplyMsg::printProtocol() {
	debug(
			"[UPLOAD_FILE_REPLY] Client ID = %" PRIu32 ", File ID = %" PRIu32 "\n",
			_clientId, _fileId);
}
