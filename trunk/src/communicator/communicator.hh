/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <list>
#include "../protocol/message.hh"
#include "connection.hh"
#include "../common/enums.hh"

using namespace std;

/**
 * Communication module for all components. Handles all sending / receiving
 * Abstract Class
 */

class Communicator {
public:

	Communicator(); // constructor
	virtual ~Communicator(); // destructor

//	void waitForMessage();

	void addConnection(string ip, uint16_t port,
			ComponentType connectionType);	// establish a connection
	void removeConnection(uint32_t sockfd);	// kill and remove connection

	// getters
	uint32_t getMdsSockfd();
	uint32_t getMonitorSockfd();

private:
	list<Connection> _mdsConnectionList;
	list<Connection> _osdConnectionList;
	list<Connection> _monitorConnectionList;
	list<Connection> _clientConnectionList;
	list<Message *> _outMessageList;	// queue of message to be sent
};
#endif
