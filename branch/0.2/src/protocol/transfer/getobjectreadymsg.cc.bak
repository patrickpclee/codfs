#include "getobjectreadymsg.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"

GetObjectReadyMsg::GetObjectReadyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetObjectReadyMsg::GetObjectReadyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_objectId = objectId;
}

void GetObjectReadyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectReadyPro getObjectReadyPro;
	getObjectReadyPro.set_objectid(_objectId);

	if (!getObjectReadyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OBJECT_READY);
	setProtocolMsg(serializedString);

}

void GetObjectReadyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectReadyPro getObjectReadyPro;
	getObjectReadyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getObjectReadyPro.objectid();

}

void GetObjectReadyMsg::doHandle() {
}

void GetObjectReadyMsg::printProtocol() {
	debug("[GET_OBJECT_READY] Object ID = %" PRIu64 "\n", _objectId);
}
