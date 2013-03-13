#include <iostream>
#include "handshakereply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "handshakerequest.hh"

HandshakeReplyMsg::HandshakeReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

HandshakeReplyMsg::HandshakeReplyMsg(Communicator* communicator, uint32_t requestId,
		uint32_t srcSockfd, uint32_t componentId, ComponentType componentType) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = srcSockfd;
	_componentId = componentId;
	_componentType = componentType;
	
}

void HandshakeReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::HandshakeReplyPro handshakeReplyPro;

	handshakeReplyPro.set_componentid(_componentId);
	handshakeReplyPro.set_componenttype(
			(ncvfs::HandshakeRequestPro_ComponentType) _componentType);

	if (!handshakeReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(HANDSHAKE_REPLY);
	setProtocolMsg(serializedString);

}

void HandshakeReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::HandshakeReplyPro handshakeReplyPro;
	handshakeReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_componentId = handshakeReplyPro.componentid();
	_componentType = (ComponentType) handshakeReplyPro.componenttype();

}

void HandshakeReplyMsg::doHandle() {
	HandshakeRequestMsg* handshakeRequestMsg =
			(HandshakeRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	handshakeRequestMsg->setTargetComponentId(_componentId);
	handshakeRequestMsg->setTargetComponentType(_componentType);
	handshakeRequestMsg->setStatus(READY);
}

void HandshakeReplyMsg::printProtocol() {
	debug("[HANDSHAKE_REPLY] Component ID = %" PRIu32 " Type = %d\n",
			_componentId, _componentType);
}
