#include <iostream>
#include <cstdio>
#include "../common/enums.hh"
#include "osd_communicator.hh"
#include "../protocol/listdirectoryrequest.hh"


using namespace std;

OsdCommunicator::OsdCommunicator() {

}

OsdCommunicator::~OsdCommunicator() {
	cout << "OSD Communicator Destroyed" << endl;
}

void OsdCommunicator::display() {
	return;
}

void OsdCommunicator::listDirectoryRequest(uint32_t osdId,
		string directoryPath) {
	// test list directory command
	printf("[List Directory] OSD: %d Path: %s\n", osdId, directoryPath.c_str());

	ListDirectoryRequestMessage* message = new ListDirectoryRequestMessage(
			osdId, directoryPath);
	message->prepareProtocolMsg();

}

void OsdCommunicator::connectToMds() {
	// test MDS connection
	string ip = "127.0.0.1";
	uint16_t port = 12345;
	ComponentType connectionType = MDS;

	addConnection(ip, port, connectionType);
}
