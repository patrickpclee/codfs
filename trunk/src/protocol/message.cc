/**
 * message.cc
 */

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "../common/enums.hh"
#include "message.hh"

using namespace std;

/**
 * Set the message type
 * @param protocolType Message type
 */

void Message::setProtocolType(MsgType protocolType) {
	_msgHeader.protocolMsgType = protocolType;
}

/**
 * Set the size of the protocol part
 * @param protocolSize Size of protocol part
 */

void Message::setProtocolSize(uint32_t protocolSize) {
	_msgHeader.protocolMsgSize = protocolSize;
}

/**
 * Set the size of payload (binary data)
 * @param payloadSize Payload size
 */

void Message::setPayloadSize(uint32_t payloadSize) {
	_msgHeader.payloadSize = payloadSize;
}

/**
 * Set the embedded protocol message (serialized string)
 * @param protocolMsg Protocol message (serialized string)
 */

void Message::setProtocolMsg(string protocolMsg) {
	_protocolMsg = protocolMsg;
}

/**
 * Set destination socket descriptor
 * @param sockfd Destination socket descriptor
 */

void Message::setSockfd(uint32_t sockfd) {
	_sockfd = sockfd;
}

/**
 * Constructor
 */

Message::Message() {
	_protocolMsg = "";
	_sockfd = 0;
	_msgHeader.payloadSize = 0;
	_msgHeader.protocolMsgSize = 0;
	_msgHeader.protocolMsgType = 0;
	_payload = NULL;
}

/**
 * Destructor
 */

Message::~Message() {
	// TODO: Deallocate Payload Buffer
}

/**
 * DEBUG: Print MsgHeader content
 */

void Message::printHeader() {
	cout << "[MsgHeader] Type = " << _msgHeader.protocolMsgType << " Size = "
			<< _msgHeader.protocolMsgSize << " Payload Size = "
			<< _msgHeader.payloadSize << endl;
}

/**
 * Copy data from file to memory buffer
 * Make _payload point to the memory buffer
 * @param filepath File to send
 * @param offset Offset of file segment
 * @param length Size of file segment
 */

void Message::preparePayload(string filepath, uint32_t offset, uint32_t length) {
	// TODO: Prepare Payload

}
