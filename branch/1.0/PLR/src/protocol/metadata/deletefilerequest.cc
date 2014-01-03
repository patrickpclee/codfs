#include <iostream>
#include "deletefilerequest.hh"
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

DeleteFileRequestMsg::DeleteFileRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
DeleteFileRequestMsg::DeleteFileRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t clientId, uint32_t fileId, const string &path) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_clientId = clientId;
	_fileId = fileId;
	_path = path;
}

void DeleteFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::DeleteFileRequestPro deleteFileRequestPro;

	deleteFileRequestPro.set_clientid(_clientId);
	deleteFileRequestPro.set_fileid(_fileId);
	deleteFileRequestPro.set_path(_path);

	if (!deleteFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(DELETE_FILE_REQUEST);
	setProtocolMsg(serializedString);

}

void DeleteFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::DeleteFileRequestPro deleteFileRequestPro;
	deleteFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = deleteFileRequestPro.clientid();
	_fileId = deleteFileRequestPro.fileid();
	_path = deleteFileRequestPro.path();
}

void DeleteFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->deleteFileProcessor(_msgHeader.requestId, _sockfd, _clientId, _fileId, _path);
#endif
}

void DeleteFileRequestMsg::printProtocol() {
	debug(
			"[DELETE_FILE_REQUEST] Client ID = %" PRIu32 ", Path = %s [%" PRIu32 "]\n",
			_clientId, _path.c_str(), _fileId);
}
