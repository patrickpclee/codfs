#include "objecttransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
#include "../../client/client.hh"
extern Client* client;
#endif
ObjectTransferEndRequestMsg::ObjectTransferEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

ObjectTransferEndRequestMsg::ObjectTransferEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_msgHeader.threadPoolLevel = 1;
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

#ifdef COMPILE_FOR_CLIENT
	debug ("Start Processor for object ID = %" PRIu64 "\n", _objectId);
	client->putObjectEndProcessor (_msgHeader.requestId, _sockfd, _objectId);
	debug ("End Processor for object ID = %" PRIu64 "\n", _objectId);
#endif
}

void ObjectTransferEndRequestMsg::printProtocol() {
	debug("[OBJECT_TRANSFER_END_REQUEST] Object ID = %" PRIu64 "\n", _objectId);
}
