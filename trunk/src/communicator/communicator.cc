/**
 * communicator.cc
 */

#include <iostream>
#include "connection.hh"
#include "../common/enums.hh"
#include "communicator.hh"
#include "../protocol/message.pb.h"

using namespace std;

Communicator::Communicator() {

	cout << "Checking Protocol Buffer Version...";
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	cout << "Success" << endl;

	cout << "Communicator Initialised" << endl;
}

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::addConnection(string ip, uint16_t port,
		ComponentType connectionType) {

	// Construct a Connection object and connect to component
	Connection conn;
	const uint32_t sockfd = conn.doConnect(ip, port, connectionType);

	// Save the connection into corresponding list
	switch (connectionType) {
	case MDS:
		_mdsConnectionMap[sockfd] = conn;
		break;
	case OSD:
		_osdConnectionMap[sockfd] = conn;
		break;
	case MONITOR:
		_monitorConnectionMap[sockfd] = conn;
		break;
	case CLIENT:
		_clientConnectionMap[sockfd] = conn;
		break;
	}

	cout << "Connection Added" << endl;
}

void Communicator::removeConnection(uint32_t sockfd) {

	if (_mdsConnectionMap.count(sockfd)) {
		_mdsConnectionMap[sockfd].disconnect();
		_mdsConnectionMap.erase(sockfd);
	} else if (_osdConnectionMap.count(sockfd)) {
		_osdConnectionMap[sockfd].disconnect();
		_osdConnectionMap.erase(sockfd);
	} else if (_clientConnectionMap.count(sockfd)) {
		_clientConnectionMap[sockfd].disconnect();
		_clientConnectionMap.erase(sockfd);
	} else if (_monitorConnectionMap.count(sockfd)) {
		_monitorConnectionMap[sockfd].disconnect();
		_monitorConnectionMap.erase(sockfd);
	} else {
		cerr << "Cannot remove connection. Socket Descriptor not found."
				<< endl;
	}

}

uint32_t Communicator::getMdsSockfd() {
	// TODO: assume return first MDS
	return _mdsConnectionMap.begin()->second.getSockfd();
}

uint32_t Communicator::getMonitorSockfd() {
	// TODO: assume return first Monitor
	return _monitorConnectionMap.begin()->second.getSockfd();
}
