#include <stdio.h>
#include "osd.hh"

Osd::Osd() {
	_osdCommunicator = new OsdCommunicator();
}

Osd::~Osd() {
	delete _osdCommunicator;
}

int main (void) {

	Osd* osd = new Osd();

	printf ("OSD\n");

	delete osd;

	return 0;
}
