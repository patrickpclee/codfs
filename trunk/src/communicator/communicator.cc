/**
 * communicator.cc
 */

#include <iostream>
#include "connection.hh"
#include "../common/enums.hh"
#include "communicator.hh"
#include "../protocol/message.pb.h"

using namespace std;

/**
 * Communicator Constructor
 * Verify Protocol Buffer version
 */

Communicator::Communicator() {

	cout << "Checking Protocol Buffer Version...";
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	cout << "Success" << endl;

	cout << "Communicator Initialised" << endl;
}

/**
 * Communicator Destructor
 */

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

/**
 * Establish a connection to a component. Save the connection to list
 * @param ip Destination ip
 * @param port Destination port
 * @param Destination type: MDS/CLIENT/MONITOR/OSD
 */

void Communicator::addConnection(string ip, uint16_t port,
		ComponentType connectionType) {

	// Construct a Connection object and connect to component
	Connection conn (ip, port, connectionType);

	// Save the connection into corresponding list
	switch (connectionType) {
		case MDS:
			_mdsConnectionList.push_back(conn);
			break;
		case OSD:
			_osdConnectionList.push_back(conn);
			break;
		case MONITOR:
			_monitorConnectionList.push_back(conn);
			break;
		case CLIENT:
			_clientConnectionList.push_back(conn);
			break;
	}

	cout << "Connection Added" << endl;
}

/**
 * Get the socket descriptor of MDS
 * @return socket descriptor of MDS
 */

uint32_t Communicator::getMdsSockfd() {
	return _mdsConnectionList.front().getSockfd();
}

/**
 * Get socket descriptor of Monitor
 * @return socket descriptor of Monitor
 */

uint32_t Communicator::getMonitorSockfd() {
	return _monitorConnectionList.front().getSockfd();
}
