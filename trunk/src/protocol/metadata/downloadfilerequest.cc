/**
 * downloadfilerequest.cc
 */

#include <iostream>
#include "downloadfilerequest.hh"
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

DownloadFileRequestMsg::DownloadFileRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
DownloadFileRequestMsg::DownloadFileRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t clientId, uint32_t fileId) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_clientId = clientId;
	_fileId = fileId;
	_filePath = "";
}

/**
 * Constructor - Save parameters in private variables
 */
DownloadFileRequestMsg::DownloadFileRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t clientId, const string &filePath) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_clientId = clientId;
	_fileId = 0;
	_filePath = filePath;
}

void DownloadFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DownloadFileRequestPro downloadFileRequestPro;

	downloadFileRequestPro.set_clientid(_clientId);
	if(_fileId == 0)
		downloadFileRequestPro.set_filepath(_filePath);
	else
		downloadFileRequestPro.set_fileid(_fileId);

	if (!downloadFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(DOWNLOAD_FILE_REQUEST);
	setProtocolMsg(serializedString);

}

void DownloadFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::DownloadFileRequestPro downloadFileRequestPro;
	downloadFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = downloadFileRequestPro.clientid();
	if(downloadFileRequestPro.has_fileid()){
		_fileId = downloadFileRequestPro.fileid();
		_filePath = "";
	} else {
		_fileId = 0;
		_filePath = downloadFileRequestPro.filepath();
	}

}

void DownloadFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	if(_fileId == 0)
		mds->downloadFileProcessor(_msgHeader.requestId, _sockfd, _clientId, _filePath);
	else
		mds->downloadFileProcessor(_msgHeader.requestId, _sockfd, _clientId, _fileId);
#endif
}

void DownloadFileRequestMsg::printProtocol() {
	debug(
			"[DOWNLOAD_FILE_REQUEST] Client ID = %" PRIu32 ", File ID = %" PRIu32 " File Path = %s\n",
			_clientId, _fileId, _filePath.c_str());
}

void DownloadFileRequestMsg::setSegmentList(vector<uint64_t> segmentList) {
	_segmentList = segmentList;
}

void DownloadFileRequestMsg::setPrimaryList(vector<uint32_t> primaryList) {
	_primaryList = primaryList;
}

vector<uint64_t> DownloadFileRequestMsg::getSegmentList() {
	return _segmentList;
}

vector<uint32_t> DownloadFileRequestMsg::getPrimaryList() {
	return _primaryList;
}

void DownloadFileRequestMsg::setFileId(uint32_t fileId) {
	_fileId = fileId;
}

uint32_t DownloadFileRequestMsg::getFileId() {
	return _fileId;
}

void DownloadFileRequestMsg::setFilePath (string filePath) {
	_filePath = filePath;
}

string DownloadFileRequestMsg::getFilePath () {
	return _filePath;
}

void DownloadFileRequestMsg::setSize (uint64_t size) {
	_size = size;
}

uint64_t DownloadFileRequestMsg::getSize() {
	return _size;
}

void DownloadFileRequestMsg::setFileType (FileType fileType) {
	_fileType = fileType;
}

FileType DownloadFileRequestMsg::getFileType() {
	return _fileType;
}
