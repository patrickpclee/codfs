#include <stdio.h>
#include "osd.hh"

Osd::Osd() {
	cout << "OSD Created" << endl;
	_osdCommunicator = new OsdCommunicator();
}

Osd::~Osd() {
	delete _osdCommunicator;
}

OsdCommunicator* Osd::getOsdCommunicator() {
	return _osdCommunicator;
}

int main(void) {

	Osd* osd = new Osd();
	OsdCommunicator* communicator = osd->getOsdCommunicator();

	communicator->connectToMds();

	// test list directory
	communicator->listDirectoryRequest(1, "/");

	delete osd;

	cout << "OSD Destroyed" << endl;

	return 0;
}
