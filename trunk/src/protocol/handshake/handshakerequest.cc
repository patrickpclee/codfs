#include <iostream>
#include "handshakerequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

HandshakeRequestMsg::HandshakeRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

HandshakeRequestMsg::HandshakeRequestMsg(Communicator* communicator,
		uint32_t srcSockfd, uint32_t componentId, ComponentType componentType) :
		Message(communicator) {

	_sockfd = srcSockfd;
	_componentId = componentId;
	_componentType = componentType;
}

void HandshakeRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::HandshakeRequestPro handshakeRequestPro;

	handshakeRequestPro.set_componentid(_componentId);
	handshakeRequestPro.set_componenttype(
			(ncvfs::HandshakeRequestPro_ComponentType) _componentType);

	if (!handshakeRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(HANDSHAKE_REQUEST);
	setProtocolMsg(serializedString);

}

void HandshakeRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::HandshakeRequestPro handshakeRequestPro;
	handshakeRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_componentId = handshakeRequestPro.componentid();
	_componentType = (ComponentType) handshakeRequestPro.componenttype();

}

void HandshakeRequestMsg::doHandle() {
	_communicator->handshakeRequestProcessor(_msgHeader.requestId, _sockfd,
			_componentId, _componentType);
}

void HandshakeRequestMsg::printProtocol() {
	debug("[HANDSHAKE_REQUEST] Component ID = %" PRIu32 "Type = %d\n",
			_componentId, _componentType);
}

void HandshakeRequestMsg::setTargetComponentId(uint32_t componentId) {
	_targetComponentId = componentId;
}

uint32_t HandshakeRequestMsg::getTargetComponentId() {
	return _targetComponentId;
}

void HandshakeRequestMsg::setTargetComponentType(ComponentType componentType) {
	_targetComponentType = componentType;
}

ComponentType HandshakeRequestMsg::getTargetComponentType() {
	return _targetComponentType;
}
