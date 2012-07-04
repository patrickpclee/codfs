#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include "../communicator/communicator.hh"

using namespace std;

class OsdCommunicator: public Communicator {
public:
	void display();
	~OsdCommunicator();
private:
};

#endif
