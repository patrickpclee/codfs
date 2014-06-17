#include <iostream>
using namespace std;

#include "renamefilerequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

RenameFileRequestMsg::RenameFileRequestMsg(Communicator* communicator) :
		Message(communicator){

}

RenameFileRequestMsg::RenameFileRequestMsg(Communicator* communicator,
		uint32_t sockfd, uint32_t clientId, uint32_t fileId, const string& path, const string& newPath) :
		Message(communicator){

	_sockfd = sockfd;
	_clientId = clientId;
	_fileId = fileId;
	_path = path;
	_newPath = newPath;
}

void RenameFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RenameFileRequestPro renameFileRequestPro;
	renameFileRequestPro.set_clientid(_clientId);
	if(_fileId > 0)
		renameFileRequestPro.set_fileid(_fileId);
	else
		renameFileRequestPro.set_path(_path);
	renameFileRequestPro.set_newpath(_newPath);

	if (!renameFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(RENAME_FILE_REQUEST);
	setProtocolMsg(serializedString);

}

void RenameFileRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RenameFileRequestPro renameFileRequestPro;
	renameFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = renameFileRequestPro.clientid();
	if(renameFileRequestPro.has_fileid()){
		_fileId = renameFileRequestPro.fileid();
		_path = "";
	} else {
		_fileId = 0;
		_path = renameFileRequestPro.path();
	}
	_newPath = renameFileRequestPro.newpath();
}

void RenameFileRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->renameFileProcessor (_msgHeader.requestId, _sockfd, _clientId, _fileId, _path, _newPath);
#endif
}

void RenameFileRequestMsg::printProtocol() {
	debug("[RENAME_FILE_REQUEST] File ID = %" PRIu32 " %s %s\n", _fileId, _path.c_str(), _newPath.c_str());
}
