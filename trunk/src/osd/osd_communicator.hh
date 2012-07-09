/**
 * osd_communicator.hh
 */

#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "../communicator/communicator.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all OSD communications
 */

class OsdCommunicator: public Communicator {
public:
	OsdCommunicator();
	~OsdCommunicator();
//	void listDirectoryRequest(uint32_t osdId, string directoryPath);
	void connectToMds ();
private:
};

#endif
