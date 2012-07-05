#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "message.hh"

using namespace std;

void Message::preparePayload (char* buf, uint32_t offset, uint32_t length) {
	if (length > PAYLOADSIZE) {
		cout << "payload size error" << endl;
		return;
	}
	_payload = (char *)malloc(sizeof(char)*length);
	memcpy (_payload, buf + offset, length);
}

void Message::prepareMsgHeader(uint32_t protocolMsgType, uint32_t protocolMsgSize) {
	_msgHeader.protocolMsgType = protocolMsgType;
	_msgHeader.protocolMsgSize = protocolMsgSize;
}

Message::Message() {
	_protocolMsg = NULL;
	_payload = NULL;
	_msgHeader.payloadSize = 0;
	_msgHeader.protocolMsgSize = 0;
	_msgHeader.protocolMsgType = 0;
}

Message::~Message() {
	delete (_protocolMsg);
	delete (_payload);
}
