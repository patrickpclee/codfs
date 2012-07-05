#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <list>
#include "connection.hh"
#include "../common/enums.hh"

using namespace std;

class Communicator {
public:
	Communicator();
	void waitForMessage();
	virtual void display() = 0; // abstract class
	virtual ~Communicator();

	void addConnection(string ip, uint16_t port, ComponentType connectionType);
	void removeConnection(uint32_t connId);
private:
	list <Connection> _connectionList;
};
#endif
