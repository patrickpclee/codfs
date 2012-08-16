#include "objecttransferendreply.hh"
#include "objecttransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../osd/osd.hh"

ObjectTransferEndReplyMsg::ObjectTransferEndReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

ObjectTransferEndReplyMsg::ObjectTransferEndReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t objectId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;
	_objectId = objectId;
}

void ObjectTransferEndReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ObjectTransferEndReplyPro objectTransferEndReplyPro;
	objectTransferEndReplyPro.set_objectid(_objectId);

	if (!objectTransferEndReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OBJECT_TRANSFER_END_REPLY);
	setProtocolMsg(serializedString);

}

void ObjectTransferEndReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ObjectTransferEndReplyPro objectTransferEndReplyPro;
	objectTransferEndReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = objectTransferEndReplyPro.objectid();

}

void ObjectTransferEndReplyMsg::doHandle() {
	ObjectTransferEndRequestMsg* putObjectEndRequestMsg =
			(ObjectTransferEndRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putObjectEndRequestMsg->setStatus(READY);
}

void ObjectTransferEndReplyMsg::printProtocol() {
	debug("[OBJECT_TRANSFER_END_REPLY] Object ID = %" PRIu64 "\n", _objectId);
}
