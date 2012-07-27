/**
 * message.cc
 */

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ios>
#include "../communicator/communicator.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "message.hh"

using namespace std;

Message::Message() {

}

Message::Message(Communicator* communicator) {

	_communicator = communicator; // needed by communicator->findWaitReplyMessage()
	_protocolMsg = "";
	_sockfd = 0;
	_msgHeader.payloadSize = 0;
	_msgHeader.protocolMsgSize = 0;
	_msgHeader.protocolMsgType = DEFAULT;
	_payload = NULL;
}

Message::~Message() {
	if (_payload != NULL) {
		MemoryPool::getInstance().poolFree(_payload);
	}
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

void Message::printHeader() {
	cout << "[MsgHeader] Type = " << _msgHeader.protocolMsgType << " Size = "
			<< _msgHeader.protocolMsgSize << " Payload Size = "
			<< _msgHeader.payloadSize << endl;
}

uint32_t Message::preparePayload(string filepath, uint32_t offset,
		uint32_t length) {

	ifstream file;

	_Ios_Iostate exceptionMask = ifstream::eofbit | ifstream::failbit
			| ifstream::badbit;
	file.exceptions(exceptionMask);

	try {

		// open file
		_Ios_Openmode openMode = ios::in | ios::binary;
		file.open(filepath.c_str(), openMode);

		// seek file
		file.seekg(offset, ios_base::beg);

		// allocate memory:
		_payload = MemoryPool::getInstance().poolMalloc(length);

		// read data as a block:
		file.read(_payload, length);

	} catch (ifstream::failure &e) {
		cerr << "Exception reading file" << endl;
	}

	// close file
	file.close();

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
