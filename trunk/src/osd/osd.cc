#include <stdio.h>
#include "osd.hh"

Osd::Osd() {
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

	// test list directory
	communicator->listDirectoryRequest(1, "/");

	delete osd;

	return 0;
}
