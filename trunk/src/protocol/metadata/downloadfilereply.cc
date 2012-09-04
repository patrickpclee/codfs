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

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
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
		uint32_t requestId, uint32_t sockfd, uint32_t fileId, string filePath, uint64_t fileSize,
		string checksum, vector<uint64_t> objectList, vector<uint32_t> primaryList) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_fileId = fileId;
	_filePath = filePath;
	_fileSize = fileSize;
	_checksum = checksum;
	_objectList = objectList;
	_primaryList = primaryList;
	_msgHeader.threadPoolLevel = 1;
}

void DownloadFileReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DownloadFileReplyPro downloadFileReplyPro;

	downloadFileReplyPro.set_fileid(_fileId);
	downloadFileReplyPro.set_filepath(_filePath);
	downloadFileReplyPro.set_filesize(_fileSize);
	downloadFileReplyPro.set_checksum(_checksum);

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

	_fileId = downloadFileReplyPro.fileid();
	_filePath = downloadFileReplyPro.filepath();
	_fileSize = downloadFileReplyPro.filesize();
	_checksum = downloadFileReplyPro.checksum();

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
	downloadFileRequestMsg->setFileId(_fileId);
	downloadFileRequestMsg->setFilePath(_filePath);
	downloadFileRequestMsg->setSize(_fileSize);
	downloadFileRequestMsg->setObjectList(_objectList);
	downloadFileRequestMsg->setPrimaryList(_primaryList);
	downloadFileRequestMsg->setStatus(READY);
}

void DownloadFileReplyMsg::printProtocol() {
	debug(
			"[DOWNLOAD_FILE_REPLY] File ID = %" PRIu32 " Size = %" PRIu64 "\n", _fileId, _fileSize);
}
