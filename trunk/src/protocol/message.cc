#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "../common/enums.hh"
#include "message.hh"

using namespace std;

void Message::preparePayload(char* buf, uint32_t offset, uint32_t length) {
	/*
	 if (length > PAYLOADSIZE) {
	 cout << "payload size error" << endl;
	 return;
	 }
	 _payload = (char *)malloc(sizeof(char)*length);
	 memcpy (_payload, buf + offset, length);
	 */
}

void Message::setProtocolType(MsgType protocolType) {
	_msgHeader.protocolMsgType = protocolType;
}

void Message::setProtocolSize(uint32_t protocolSize) {
	_msgHeader.protocolMsgSize = protocolSize;
}

void Message::setPayloadSize(uint32_t payloadSize) {
	_msgHeader.payloadSize = payloadSize;
}

Message::Message() {
	_protocolMsg = NULL;
	_payload = NULL;
	_msgHeader.payloadSize = 0;
	_msgHeader.protocolMsgSize = 0;
	_msgHeader.protocolMsgType = 0;
}

Message::~Message() {
	// TODO: Deallocate Memory
}

void Message::printHeader() {
	cout << "[MsgHeader] Type = " << _msgHeader.protocolMsgType << " Size = "
			<< _msgHeader.protocolMsgSize << " Payload Size = "
			<< _msgHeader.payloadSize << endl;
}
