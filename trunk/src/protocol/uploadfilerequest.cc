/**
 * uploadfilerequest.cc
 */

#include <iostream>
#include "uploadfilerequest.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../mds/mds.hh"

#ifdef COMPILE_FOR_MDS
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator,
		uint32_t clientId, uint32_t mdsSockfd, string path) :
		Message(communicator) {
	_clientId = clientId;
	_path = path;
	_sockfd = mdsSockfd;
}

void UploadFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::UploadFileRequestPro uploadFileRequestPro;
	//uploadFileRequestPro.set_directorypath(_directoryPath);
	//uploadFileRequestPro.set_osdid(_clientId);

	if (!uploadFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);
	setProtocolMsg(serializedString);

}

void UploadFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadFileRequestPro uploadFileRequestPro;
	uploadFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	//_clientId = uploadFileRequestPro.osdid();
	//_directoryPath = uploadFileRequestPro.directorypath();

}

void UploadFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	//mds->listFolderProcessor(_msgHeader.requestId,_sockfd,_clientId,_directoryPath);
#endif
}

void UploadFileRequestMsg::printProtocol() {
	debug("[UPLOAD_FILE_REQUEST] Client ID = %" PRIu32 ", Path = %s\n", _clientId,
			_path.c_str());
}

void UploadFileRequestMsg::setObjectIdList(vector<uint64_t> objectIdList) {
	_objectIdList = objectIdList;

	return;
}

void UploadFileRequestMsg::setPrimaryList(vector<uint32_t> primaryList) {
	_primaryList = primaryList;

	return;
}

void UploadFileRequestMsg::setFileId(uint32_t fileId) {
	_fileId = fileId;

	return;
}

vector<uint64_t> UploadFileRequestMsg::getObjectIdList() {
	return _objectIdList;
}

vector<uint32_t> UploadFileRequestMsg::getPrimaryList() {
	return _primaryList;
}

uint32_t UploadFileRequestMsg::getFileId() {
	return _fileId;
}
