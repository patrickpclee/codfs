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
}

void DownloadFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DownloadFileRequestPro downloadFileRequestPro;

	downloadFileRequestPro.set_clientid(_clientId);
	downloadFileRequestPro.set_fileid(_fileId);

	if (!downloadFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (DOWNLOAD_FILE_REQUEST);
	setProtocolMsg(serializedString);

}

void DownloadFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::DownloadFileRequestPro downloadFileRequestPro;
	downloadFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = downloadFileRequestPro.clientid();
	_fileId = downloadFileRequestPro.fileid();

}

void DownloadFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->downloadFileProcessor(_msgHeader.requestId, _sockfd, _clientId, _fileId);
#endif
}

void DownloadFileRequestMsg::printProtocol() {
	debug(
			"[UPLOAD_FILE_REQUEST] Client ID = %" PRIu32 ", File ID = %" PRIu32 "\n",
			_clientId, _fileId);
}

void DownloadFileRequestMsg::setObjectList(vector<uint64_t> objectList) {
	_objectList = objectList;
}

void DownloadFileRequestMsg::setPrimaryList(vector<uint32_t> primaryList) {
	_primaryList = primaryList;
}

vector<uint64_t> DownloadFileRequestMsg::getObjectList() {
	return _objectList;
}

vector<uint32_t> DownloadFileRequestMsg::getPrimaryList() {
	return _primaryList;
}
