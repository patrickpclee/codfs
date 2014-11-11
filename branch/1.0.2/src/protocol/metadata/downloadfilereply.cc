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
		uint32_t requestId, uint32_t sockfd, uint32_t fileId, const string &filePath, uint64_t fileSize, const FileType& fileType, const vector<uint64_t> &segmentList, const vector<uint32_t> &primaryList) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_fileId = fileId;
	_filePath = filePath;
	_fileSize = fileSize;
	_fileType = fileType;
	_segmentList = segmentList;
	_primaryList = primaryList;
	
}

void DownloadFileReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DownloadFileReplyPro downloadFileReplyPro;

	downloadFileReplyPro.set_fileid(_fileId);
	downloadFileReplyPro.set_filepath(_filePath);
	downloadFileReplyPro.set_filesize(_fileSize);
	downloadFileReplyPro.set_filetype((ncvfs::DownloadFileReplyPro_FileType)_fileType);

	for (auto segmentID : _segmentList) {
		downloadFileReplyPro.add_segmentlist(segmentID);
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
	_fileType = (FileType)downloadFileReplyPro.filetype();

	for (int i = 0; i < downloadFileReplyPro.segmentlist_size(); i++) {
		_segmentList.push_back(downloadFileReplyPro.segmentlist(i));
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
	downloadFileRequestMsg->setSegmentList(_segmentList);
	downloadFileRequestMsg->setPrimaryList(_primaryList);
	downloadFileRequestMsg->setStatus(READY);
}

void DownloadFileReplyMsg::printProtocol() {
	debug(
			"[DOWNLOAD_FILE_REPLY] File ID = %" PRIu32 " Size = %" PRIu64 "\n", _fileId, _fileSize);
}
