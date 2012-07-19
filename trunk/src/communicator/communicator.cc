/**
 * communicator.cc
 */

#include <iostream>
#include <thread>
#include <sys/types.h>		// required by select()
#include <unistd.h>		// required by select()
#include <sys/select.h>	// required by select()
#include "connection.hh"
#include "communicator.hh"
#include "../config/config.hh"
#include "../common/enums.hh"
#include "../protocol/message.pb.h"
#include "../protocol/messagefactory.hh"

using namespace std;

// global variable defined in each component
extern ConfigLayer* configLayer;

Communicator::Communicator() {

	cout << "Checking Protocol Buffer Version...";
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	cout << "Success" << endl;

	cout << "Communicator Initialised" << endl;
}

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::waitForMessage() {
	char* buf;
	uint32_t maxFd;
	fd_set sockfdSet;
	struct timeval tv; // timeout for select
	map<uint32_t, Connection>::iterator p;

	// get the maximum socket descriptor in the connectionMap
	maxFd = _connectionMap.rbegin()->first;

	// set select timeout value
	tv.tv_sec = configLayer->getConfigInt("Communication>SelectTimeout>sec");
	tv.tv_usec = configLayer->getConfigInt("Communication>SelectTimeout>usec");

	cout << "[WAIT FOR MESSAGE] Select Timeout = " << tv.tv_sec << " "
			<< tv.tv_usec << endl;
	cout << "[WAIT FOR MESSAGE] Ready" << endl;

	while (1) {

		FD_ZERO(&sockfdSet);

		// add all socket descriptors into sockfdSet
		for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
			FD_SET(p->second.getSockfd(), &sockfdSet);
		}

		// invoke select
		if (select(maxFd + 1, &sockfdSet, NULL, NULL, &tv) == -1) {
			cerr << "select error" << endl;
			return;
		}

		cout << "[WAIT FOR MESSAGE] Select returns" << endl;

		for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
			cout << "[WAIT FOR MESSAGE] Checking sockfd = " << p->first << endl;
			// if socket has data available
			if (FD_ISSET(p->second.getSockfd(), &sockfdSet)) {
				// receive message into buffer, memory allocated in recvMessage
				buf = p->second.recvMessage();
				dispatch(buf, p->first);
			}
		}

		// TODO: ping all nodes after timeout
		// pingAllConnections();
		cout << "[WAIT FOR MESSAGE] Checking connections..." << endl;

	} // end while (1)
}

void Communicator::sendMessage() {

	// get config: polling interval
	const uint32_t pollingInterval = configLayer->getConfigInt(
			"Communication>SendPollingInterval");

	// TODO: poll outMessageQueue to send message for now
	while (1) {

		// send all message in the outMessageQueue
		while (!_outMessageQueue.empty()) {
			Message* message = _outMessageQueue.front();
			_outMessageQueue.pop_front();
			const uint32_t sockfd = message->getSockfd();
			_connectionMap[sockfd].sendMessage(message);
		}

		sleep(pollingInterval);
	}

}

/**
 * 1. Connect to target component
 * 2. Add the connection to the corresponding map
 */

void Communicator::addConnection(string ip, uint16_t port,
		ComponentType connectionType) {

	// Construct a Connection object and connect to component
	Connection conn;
	const uint32_t sockfd = conn.doConnect(ip, port, connectionType);

	// Single Thread Only
	// Save the connection into corresponding list
	_connectionMap[sockfd] = conn;

	cout << "Connection Added" << endl;
}

/**
 * 1. Disconnect the connection
 * 2. Remove the connection from map
 */

void Communicator::removeConnection(uint32_t sockfd) {

	if (_connectionMap.count(sockfd)) {
		_connectionMap[sockfd].disconnect();
		_connectionMap.erase(sockfd);
	} else {
		cerr << "Connection not found, cannot remove connection" << endl;
	}

}

uint32_t Communicator::getMdsSockfd() {
	// TODO: assume return first MDS
	map<uint32_t, Connection>::iterator p;

	for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
		if (p->second.getConnectionType() == MDS) {
			return p->second.getSockfd();
		}
	}

	return -1;
}

uint32_t Communicator::getMonitorSockfd() {
	// TODO: assume return first Monitor
	map<uint32_t, Connection>::iterator p;

	for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
		if (p->second.getConnectionType() == MONITOR) {
			return p->second.getSockfd();
		}
	}

	return -1;
}

/**
 * 1. Get the MsgHeader from the receive buffer
 * 2. Get the MsgType from the MsgHeader
 * 3. Use the MessageFactory to obtain a new Message object
 * 4. Fill in the socket descriptor into the Message
 * 5. message->parse()
 * 6. start new thread for message->handle()
 */

void Communicator::handleThread(Message* message) {
	message->handle();
}

void Communicator::dispatch(char* buf, uint32_t sockfd) {
	struct MsgHeader msgHeader;
	memcpy(&msgHeader, buf, sizeof(struct MsgHeader));

	const MsgType msgType = msgHeader.protocolMsgType;

	Message* message = MessageFactory::createMessage(msgType);
	message->setSockfd(sockfd);
	message->parse(buf);

	thread t(handleThread, message);
	t.detach();

	// TODO: when to free message?
}
