#include <iostream>
#include <string.h>
#include <stdio.h>
#include "connection.hh"
#include "socket.hh"
#include "../common/enums.hh"
#include "../common/debug.hh"
#include "../protocol/message.hh"
#include "../common/memorypool.hh"
#include "socketexception.hh"

// debug
#include "../protocol/message.pb.h"
#include "../common/debug.hh"

using namespace std;

Connection::Connection() {

}

Connection::Connection(string ip, uint16_t port, ComponentType connectionType) {
	doConnect(ip, port, connectionType);
}

uint32_t Connection::doConnect(string ip, uint16_t port,
		ComponentType connectionType) {

	if (!_socket.create()) {
		throw SocketException("Could not create client socket.");
	}

	if (!_socket.connect(ip, (int) port)) {
		throw SocketException("Could not bind to port.");
	}

	debug("Connected to sockfd = %" PRIu32 "\n", _socket.getSockfd());

	// save connection info in private variables
	_connectionType = connectionType;

	_isDisconnected = false;

	return _socket.getSockfd();
}

uint32_t Connection::sendMessage(Message* message) {
	int32_t byteSent = 0;
	uint32_t totalByteSent = 0;

	// send header

	struct MsgHeader msgHeader = message->getMsgHeader();
	const uint32_t headerLength = sizeof(struct MsgHeader);

	if ((byteSent = _socket.sendn((const char*) &msgHeader, headerLength))
			!= -1) {
		totalByteSent += byteSent;
	} else {
		cerr << "Error in sending MsgHeader" << endl;
	}

	debug("ID: %" PRIu32 " Header sent %" PRIu32 " bytes\n",
			message->getMsgHeader().requestId, byteSent);

	// send protocol message

	byteSent = 0;

	string protocolMessage = message->getProtocolMsg();
	const uint32_t protocolLength = protocolMessage.length();

	if ((byteSent = _socket.sendn(protocolMessage.data(), protocolLength))
			!= -1) {
		totalByteSent += byteSent;
	} else {
		cerr << "Error in sending Protocol Message" << endl;
	}

	debug("ID: %" PRIu32 " Protocol sent %" PRIu32 " bytes\n",
			message->getMsgHeader().requestId, byteSent);

	// send payload if there is one

	byteSent = 0;

	if (msgHeader.payloadSize > 0) {
		char* payload = message->getPayload();
		const uint32_t payloadLength = msgHeader.payloadSize;

		if ((byteSent = _socket.sendn(payload, payloadLength)) != -1) {
			totalByteSent += byteSent;
		} else {
			cerr << "Error in sending Protocol Message" << endl;
		}

	}

	debug("ID: %" PRIu32 " Payload sent %" PRIu32 " bytes\n",
			message->getMsgHeader().requestId, byteSent);

	return totalByteSent;
}

char* Connection::recvMessage() {
	char* buf;
	int32_t byteReceived = 0;
	int32_t totalByteReceived = 0;

	// receive header

	struct MsgHeader msgHeader;
	const uint32_t headerLength = sizeof(struct MsgHeader);

	if ((byteReceived = _socket.recvn((char*) &msgHeader, headerLength))
			!= -1) {
		totalByteReceived += byteReceived;
	} else {
		cerr << "Error in receiving MsgHeader" << endl;
	}

	debug("Header received %" PRIu32 " bytes\n", byteReceived);

	// allocate buffer

	const uint32_t bufferSize = headerLength + msgHeader.protocolMsgSize
			+ msgHeader.payloadSize;

	// poolFree in message.cc->handle()
	buf = MemoryPool::getInstance().poolMalloc(bufferSize);

	// copy header to buffer
	memcpy(buf, &msgHeader, headerLength);

	debug("Type %d\n", (int)msgHeader.protocolMsgType);
	debug("Buffer Size %" PRIu32 "\n", bufferSize);
	// receive protocol message

	const uint32_t protocolLength = msgHeader.protocolMsgSize;

	byteReceived = 0;
	if ((byteReceived = _socket.recvn(buf + totalByteReceived, protocolLength))
			!= -1) {
		totalByteReceived += byteReceived;
	} else {
		cerr << "Error in receiving protocol message" << endl;
	}

	debug("Protocol received %" PRIu32 " bytes\n", byteReceived);

	// receive payload if needed

	byteReceived = 0;
	if (msgHeader.payloadSize > 0) {
		const uint32_t payloadLength = msgHeader.payloadSize;

		if ((byteReceived = _socket.recvn(buf + totalByteReceived,
				payloadLength)) != -1) {
			totalByteReceived += byteReceived;
		} else {
			cerr << "Error in receiving payload" << endl;
		}

	}

	debug("Payload received %" PRIu32 " bytes\n", byteReceived);

	return buf;

}

void Connection::disconnect() {
	_socket.~Socket();
}

uint32_t Connection::getSockfd() {
	return _socket.getSockfd();
}

ComponentType Connection::getConnectionType() {
	return _connectionType;
}

Socket* Connection::getSocket() {
	return &_socket;
}

void Connection::setConnectionType (ComponentType type){
	_connectionType = type;
}
