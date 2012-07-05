#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "../communicator/communicator.hh"

using namespace std;

class OsdCommunicator: public Communicator {
public:
	OsdCommunicator();
	~OsdCommunicator();
	void display();
	void listDirectoryRequest(uint32_t osdId, string directoryPath);
	void connectToMds ();
private:
};

#endif
