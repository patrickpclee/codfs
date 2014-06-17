#include <iostream>
using namespace std;

#include "setfilesizerequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

SetFileSizeRequestMsg::SetFileSizeRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

SetFileSizeRequestMsg::SetFileSizeRequestMsg(Communicator* communicator,
		uint32_t sockfd, uint32_t clientId, uint32_t fileId, uint64_t fileSize) :
		Message(communicator) {

	_sockfd = sockfd;
	_clientId = clientId;
	_fileId = fileId;
	_fileSize = fileSize;
}

void SetFileSizeRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SetFileSizeRequestPro setFileSizeRequestPro;
	setFileSizeRequestPro.set_clientid(_clientId);
	setFileSizeRequestPro.set_fileid(_fileId);
	setFileSizeRequestPro.set_filesize(_fileSize);

	if (!setFileSizeRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SET_FILE_SIZE_REQUEST);
	setProtocolMsg(serializedString);

}

void SetFileSizeRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SetFileSizeRequestPro setFileSizeRequestPro;
	setFileSizeRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = setFileSizeRequestPro.clientid();
	_fileId = setFileSizeRequestPro.fileid();
	_fileSize = setFileSizeRequestPro.filesize();
}

void SetFileSizeRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->setFileSizeProcessor (_msgHeader.requestId, _sockfd, _clientId, _fileId, _fileSize);
#endif
}

void SetFileSizeRequestMsg::printProtocol() {
	debug("[SET_FILE_SIZE_REQUEST] File ID = %" PRIu32 ", File Size = %" PRIu64 "\n", _fileId, _fileSize);
}
