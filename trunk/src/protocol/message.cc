/**
 * message.cc
 */

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ios>
#include "../communicator/communicator.hh"
#include "../common/debug.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "message.hh"

using namespace std;

Message::Message() {

}

Message::Message(Communicator* communicator) {

	_sockfd = 0;
	memset (&_msgHeader, 0, sizeof (struct MsgHeader));
	_protocolMsg = "";
	_payload = NULL;
	_recvBuf = NULL;
	_communicator = communicator; // needed by communicator->findWaitReplyMessage()
}

Message::~Message() {
	debug ("%s\n", "message destructor");
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

void Message::setProtocolMsg(string protocolMsg) {
	_protocolMsg = protocolMsg;
}

void Message::setSockfd(uint32_t sockfd) {
	_sockfd = sockfd;
}

void Message::setRequestId(uint32_t requestId) {
	_msgHeader.requestId = requestId;
}

void Message::setPayload(char* payload) {
	_payload = payload;
}

void Message::setRecvBuf(char* recvBuf) {
	_recvBuf = recvBuf;
}

void Message::printHeader() {
	debug("[MsgHeader] Type = %d Size = %d, PayloadSize = %d\n",
			_msgHeader.protocolMsgType, _msgHeader.protocolMsgSize, _msgHeader.payloadSize);
}

void Message::printPayloadHex() {
	printhex(_payload, _msgHeader.payloadSize);
}

uint32_t Message::preparePayload(char* buf, uint32_t length) {

	_payload = buf;
	_msgHeader.payloadSize = length;

	return 0;
}

struct MsgHeader Message::getMsgHeader() {
	return _msgHeader;
}

string Message::getProtocolMsg() {
	return _protocolMsg;
}

char* Message::getPayload() {
	return _payload;
}

uint32_t Message::getSockfd() {
	return _sockfd;
}

MessageStatus Message::waitForStatusChange() {
	return _status.get_future().get();
}

void Message::setStatus(MessageStatus status) {
	_status.set_value(status);
	return;
}

void Message::handle() {
	// message-specific handler
	doHandle();

	// cleanup
	MemoryPool::getInstance().poolFree(_recvBuf);
	delete this;
}
