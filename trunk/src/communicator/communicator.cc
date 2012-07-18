/**
 * communicator.cc
 */

#include <iostream>
#include "connection.hh"
#include "../common/enums.hh"
#include "communicator.hh"
#include "../protocol/message.pb.h"
#include "../config/config.hh"

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
	// TODO: wait for message

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

		sleep (pollingInterval);
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

	// CAUTION: Single Thread Only
	// Save the connection into corresponding list
	// Delete existing connection if sockfd present
	if (_connectionMap.count(sockfd)) {
		_connectionMap.erase(sockfd);
	}
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
