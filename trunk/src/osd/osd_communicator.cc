/**
 * osd_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "osd_communicator.hh"
#include "../common/enums.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../common/debug.hh"

using namespace std;

/**
 * Constructor
 */

OsdCommunicator::OsdCommunicator() {

}

/**
 * Destructor
 */

OsdCommunicator::~OsdCommunicator() {

}

/**
 * Establish connection to MDS
 */

void OsdCommunicator::connectToMds() {

	// example
	string ip = "127.0.0.1";
	uint16_t port = 30000;
	ComponentType connectionType = MDS;

	// do connection
	connectAndAdd(ip, port, connectionType);
}

