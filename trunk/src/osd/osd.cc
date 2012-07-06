/**
 * osd.cc
 */

#include <stdio.h>
#include "osd.hh"

/**
 * OSD Constructor
 * Initialise a communicator
 */

Osd::Osd() {
	cout << "OSD Created" << endl;
	_osdCommunicator = new OsdCommunicator();
}

/**
 * OSD Destructor
 */
Osd::~Osd() {
	delete _osdCommunicator;
}

/**
 * Get the OSD Communicator
 * @return Pointer to OSD Communicator
 */
OsdCommunicator* Osd::getOsdCommunicator() {
	return _osdCommunicator;
}

/**
 * Main function
 * @return 0 if success;
 */

int main(void) {

	// create new OSD object
	Osd* osd = new Osd();

	// create new communicator
	OsdCommunicator* communicator = osd->getOsdCommunicator();

	// connect to MDS
	communicator->connectToMds();

	// TEST: list directory from MDS
	communicator->listDirectoryRequest(1, "/");


	// cleanup
	delete osd;
	cout << "OSD Destroyed" << endl;

	return 0;
}
