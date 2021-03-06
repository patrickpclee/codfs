#include "putobjectinitreply.hh"
#include "putobjectinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

PutObjectInitReplyMsg::PutObjectInitReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectInitReplyMsg::PutObjectInitReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_objectId = objectId;
	
}

void PutObjectInitReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectInitReplyPro putObjectInitReplyPro;
	putObjectInitReplyPro.set_objectid(_objectId);

	if (!putObjectInitReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_OBJECT_INIT_REPLY);
	setProtocolMsg(serializedString);

}

void PutObjectInitReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectInitReplyPro putObjectInitReplyPro;
	putObjectInitReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putObjectInitReplyPro.objectid();

}

void PutObjectInitReplyMsg::doHandle() {
	PutObjectInitRequestMsg* putObjectInitRequestMsg =
			(PutObjectInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putObjectInitRequestMsg->setStatus(READY);
}

void PutObjectInitReplyMsg::printProtocol() {
	debug("[PUT_OBJECT_INIT_REPLY] Object ID = %" PRIu64 "\n", _objectId);
}
