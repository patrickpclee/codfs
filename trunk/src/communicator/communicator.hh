#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include "connection.hh"

class Communicator {
public:
	Communicator();
	void waitForMessage();
	virtual void display() = 0; // abstract class
	virtual ~Communicator();
private:
};
#endif
