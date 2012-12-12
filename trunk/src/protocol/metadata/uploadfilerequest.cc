/**
 * uploadfilerequest.cc
 */

#include <iostream>
#include "uploadfilerequest.hh"
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

UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t clientId, const string &path, uint64_t fileSize,
		uint32_t numOfObjs, CodingScheme codingScheme, const string &codingSetting) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_clientId = clientId;
	_path = path;
	_fileSize = fileSize;
	_numOfObjs = numOfObjs;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
}

void UploadFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::UploadFileRequestPro uploadFileRequestPro;

	uploadFileRequestPro.set_clientid(_clientId);
	uploadFileRequestPro.set_path(_path);
	uploadFileRequestPro.set_filesize(_fileSize);
	uploadFileRequestPro.set_numofobjs(_numOfObjs);
	uploadFileRequestPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	uploadFileRequestPro.set_codingsetting(_codingSetting);

	if (!uploadFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(UPLOAD_FILE_REQUEST);
	setProtocolMsg(serializedString);

}

void UploadFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadFileRequestPro uploadFileRequestPro;
	uploadFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = uploadFileRequestPro.clientid();
	_path = uploadFileRequestPro.path();
	_fileSize = uploadFileRequestPro.filesize();
	_numOfObjs = uploadFileRequestPro.numofobjs();
	_codingScheme = (CodingScheme) uploadFileRequestPro.codingscheme();
	_codingSetting = uploadFileRequestPro.codingsetting();

}

void UploadFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->uploadFileProcessor(_msgHeader.requestId, _sockfd, _clientId, _path,
			_fileSize, _numOfObjs, _codingScheme, _codingSetting);
#endif
}

void UploadFileRequestMsg::printProtocol() {
	debug(
			"[UPLOAD_FILE_REQUEST] Client ID = %" PRIu32 ", Path = %s, CodingScheme = %d, CodingSetting = %s\n",
			_clientId, _path.c_str(), _codingScheme, _codingSetting.c_str());
}

void UploadFileRequestMsg::setSegmentList(vector<uint64_t> segmentList) {
	_segmentList = segmentList;

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

vector<uint64_t> UploadFileRequestMsg::getSegmentList() {
	return _segmentList;
}

vector<uint32_t> UploadFileRequestMsg::getPrimaryList() {
	return _primaryList;
}

uint32_t UploadFileRequestMsg::getFileId() {
	return _fileId;
}
