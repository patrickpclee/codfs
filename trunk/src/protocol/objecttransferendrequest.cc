#include "objecttransferendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

ObjectTransferEndRequestMsg::ObjectTransferEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

ObjectTransferEndRequestMsg::ObjectTransferEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
}

void ObjectTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ObjectTransferEndRequestPro objectTransferEndRequestPro;
	objectTransferEndRequestPro.set_objectid(_objectId);

	if (!objectTransferEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OBJECT_TRANSFER_END_REQUEST);
	setProtocolMsg(serializedString);

}

void ObjectTransferEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ObjectTransferEndRequestPro objectTransferEndRequestPro;
	objectTransferEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = objectTransferEndRequestPro.objectid();

}

void ObjectTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectEndProcessor (_msgHeader.requestId, _sockfd, _objectId);
#endif
}

void ObjectTransferEndRequestMsg::printProtocol() {
	debug("[OBJECT_TRANSFER_END_REQUEST] Object ID = %" PRIu64 "\n", _objectId);
}
