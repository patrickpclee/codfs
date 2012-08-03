#include "putobjectendreply.hh"
#include "putobjectendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../osd/osd.hh"

PutObjectEndReplyMsg::PutObjectEndReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectEndReplyMsg::PutObjectEndReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t objectId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;
	_objectId = objectId;
}

void PutObjectEndReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectEndReplyPro putObjectEndReplyPro;
	putObjectEndReplyPro.set_objectid(_objectId);

	if (!putObjectEndReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_OBJECT_END_REPLY);
	setProtocolMsg(serializedString);

}

void PutObjectEndReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectEndReplyPro putObjectEndReplyPro;
	putObjectEndReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putObjectEndReplyPro.objectid();

}

void PutObjectEndReplyMsg::handle() {
	PutObjectEndRequestMsg* putObjectEndRequestMsg =
			(PutObjectEndRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putObjectEndRequestMsg->setStatus(READY);
	return;
}

void PutObjectEndReplyMsg::printProtocol() {
	debug("[PUT_OBJECT_END_REPLY] Object ID = %lu\n", _objectId);
}
