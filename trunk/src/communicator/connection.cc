#include <iostream>
#include <sys/socket.h>		// for socket() and connect()
#include <sys/types.h>			// for connect()
#include <netinet/in.h>		// for struct sockaddr_in
#include <arpa/inet.h>			// for ip conversion
#include <netdb.h>				// for gethostbyname()
#include <string.h>
#include <stdio.h>
#include "connection.hh"
#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "../common/memorypool.hh"

using namespace std;

Connection::Connection() {

}

Connection::Connection(string ip, uint16_t port, ComponentType connectionType) {
	doConnect(ip, port, connectionType);
}

uint32_t Connection::doConnect(string ip, uint16_t port,
		ComponentType connectionType) {
	struct sockaddr_in servaddr; // connection struct
	uint32_t sockfd; // resulting sockfd
	uint32_t co; // connection result
	struct hostent *ht; // lookup result

	printf("Connecting to %s:%d (Type: %d)\n", ip.c_str(), port,
			connectionType);

	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// fill up destination info in servaddr
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	ht = gethostbyname(ip.c_str());

	if (ht == NULL) {
		cerr << "ERROR: invalid host";
		return -1;
	}

	memcpy(&servaddr.sin_addr, ht->h_addr, ht->h_length);

	// establish connection
	co = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (co < 0) {
		cerr << "ERROR: connect failed";
		return -1;
	}

	printf("Connected to sockfd = %d\n", sockfd);

	// save connection info in private variables
	_sockfd = sockfd;
	_ip = ip;
	_port = port;
	_connectionType = connectionType;

	return sockfd;
}

uint32_t Connection::sendMessage(Message* message) {
	const uint32_t sockfd = _sockfd;
	uint32_t byteSent = 0;
	uint32_t totalByteSent = 0;

	// send header

	struct MsgHeader msgHeader = message->getMsgHeader();
	const uint32_t headerLength = sizeof(struct MsgHeader);

	if ((byteSent = sendn(sockfd, &msgHeader, headerLength)) != -1) {
		totalByteSent += byteSent;
	} else {
		cerr << "Error in sending MsgHeader" << endl;
	}

	// send protocol message

	string protocolMessage = message->getProtocolMsg();
	const uint32_t protocolLength = protocolMessage.length();

	if ((byteSent = sendn(sockfd, protocolMessage.data(), protocolLength))
			!= -1) {
		totalByteSent += byteSent;
	} else {
		cerr << "Error in sending Protocol Message" << endl;
	}

	// send payload if there is one

	if (msgHeader.payloadSize > 0) {
		char* payload = message->getPayload();
		const uint32_t payloadLength = msgHeader.payloadSize;

		if ((byteSent = sendn(sockfd, payload, payloadLength)) != -1) {
			totalByteSent += byteSent;
		} else {
			cerr << "Error in sending Protocol Message" << endl;
		}

	}

	return totalByteSent;
}

char* Connection::recvMessage() {
	const uint32_t sockfd = _sockfd;
	char* buf;
	uint32_t byteReceived = 0;
	uint32_t totalByteReceived = 0;

	// receive header

	struct MsgHeader msgHeader;
	const uint32_t headerLength = sizeof(struct MsgHeader);

	if ((byteReceived = recvn(sockfd, &msgHeader, headerLength)) != -1) {
		totalByteReceived += byteReceived;
	} else {
		cerr << "Error in receiving MsgHeader" << endl;
	}

	// allocate buffer

	const uint32_t bufferSize = headerLength + msgHeader.protocolMsgSize
			+ msgHeader.payloadSize;
	buf = MemoryPool::getInstance().poolMalloc(bufferSize);

	// TODO: free buf

	// copy header to buffer
	memcpy(buf, &msgHeader, headerLength);

	// receive protocol message

	const uint32_t protocolLength = msgHeader.protocolMsgSize;

	if ((byteReceived = recvn(sockfd, buf + totalByteReceived, protocolLength))
			!= -1) {
		totalByteReceived += byteReceived;
	} else {
		cerr << "Error in receiving protocol message" << endl;
	}

	// receive payload if needed

	if (msgHeader.payloadSize > 0) {
		const uint32_t payloadLength = msgHeader.payloadSize;

		if ((byteReceived = recvn(sockfd, buf + totalByteReceived,
				payloadLength)) != -1) {
			totalByteReceived += byteReceived;
		} else {
			cerr << "Error in receiving payload" << endl;
		}

	}

	return buf;

}

void Connection::disconnect() {
	close(_sockfd);
}

uint32_t Connection::getSockfd() {
	return _sockfd;
}

//
// PRIVATE METHODS
//

uint32_t sendn(uint32_t sd, const char* buf, uint32_t buf_len) {
	uint32_t n_left = buf_len; // actual data bytes sent
	uint32_t n;
	while (n_left > 0) {
		if ((n = send(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

uint32_t recvn(uint32_t sd, char* buf, uint32_t buf_len) {
	uint32_t n_left = buf_len;
	uint32_t n = 0;
	while (n_left > 0) {
		if ((n = recv(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}
