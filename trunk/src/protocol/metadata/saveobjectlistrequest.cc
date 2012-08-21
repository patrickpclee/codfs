#include <iostream>
using namespace std;

#include "saveobjectlistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

SaveObjectListRequestMsg::SaveObjectListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

SaveObjectListRequestMsg::SaveObjectListRequestMsg(Communicator* communicator,
		uint32_t sockfd, uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList) :
		Message(communicator) {

	_sockfd = sockfd;
	_clientId = clientId;
	_fileId = fileId;
	_objectList = objectList;
}

void SaveObjectListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SaveObjectListRequestPro saveObjectListRequestPro;
	saveObjectListRequestPro.set_clientid(_clientId);
	saveObjectListRequestPro.set_fileid(_fileId);

	for (auto objectId : _objectList) {
		saveObjectListRequestPro.add_objectlist(objectId);
	}

	if (!saveObjectListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SAVE_OBJECT_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void SaveObjectListRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SaveObjectListRequestPro saveObjectListRequestPro;
	saveObjectListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = saveObjectListRequestPro.clientid();
	_fileId = saveObjectListRequestPro.fileid();

	for (int i = 0; i < saveObjectListRequestPro.objectlist_size(); ++i) {
		_objectList.push_back(saveObjectListRequestPro.objectlist(i));
	}
}

void SaveObjectListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->saveObjectListProcessor (_msgHeader.requestId, _sockfd, _clientId, _fileId, _objectList);
#endif
}

void SaveObjectListRequestMsg::printProtocol() {
	debug("[SAVE_OBJECT_LIST_REQUEST] File ID = %" PRIu32 ", Number of Object %d\n", _fileId,_objectList.size());
}
