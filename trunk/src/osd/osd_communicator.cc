/**
 * osd_communicator.cc
 */

#include <iostream>
#include <cstdio>
#include "osd_communicator.hh"
#include "../common/enums.hh"
#include "../protocol/listdirectoryrequest.hh"

using namespace std;

/**
 * Constructor
 */

OsdCommunicator::OsdCommunicator() {
	cout << "OSD Communicator Created" << endl;

}

/**
 * Destructor
 */

OsdCommunicator::~OsdCommunicator() {
	cout << "OSD Communicator Destroyed" << endl;
}

/**
 * Request MDS for listing files in a directory
 * @param osdId My OSD ID
 * @param directoryPath Directory to list
 */

void OsdCommunicator::listDirectoryRequest(uint32_t osdId,
		string directoryPath) {

	// get socket descriptor of MDS
	const uint32_t mdsSockfd = getMdsSockfd();

	printf("[List Directory] OSD: %d Path: %s\n", osdId, directoryPath.c_str());

	// create new message
	ListDirectoryRequestMessage* message = new ListDirectoryRequestMessage(
			osdId, directoryPath, mdsSockfd);

	// prepare message
	message->prepareProtocolMsg();

	// debug: print message content
	message->printHeader();
	message->printProtocol();

	// TODO: send message
}

/**
 * Establish connection to MDS
 */

void OsdCommunicator::connectToMds() {

	// TESTING: Hardcode destination info
	// TODO: Read connection information from cache / XML
	string ip = "127.0.0.1";
	uint16_t port = 12345;
	ComponentType connectionType = MDS;

	// do connection
	addConnection(ip, port, connectionType);
}
