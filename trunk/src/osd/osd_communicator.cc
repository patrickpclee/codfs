#include <iostream>
#include <cstdio>
#include "osd_communicator.hh"
#include "../protocol/message.pb.h"

using namespace std;

OsdCommunicator::OsdCommunicator() {

}

OsdCommunicator::~OsdCommunicator(){
	cout << "OSD Communicator Destroyed" << endl;
}

void OsdCommunicator::display(){
	return;
}

void OsdCommunicator::listDirectoryRequest(uint32_t osdId, string directoryPath) {
	printf ("=== List Directory ===\n");
	printf ("OSD %d: %s\n", osdId, directoryPath.c_str());
	printf ("======================\n");
}
