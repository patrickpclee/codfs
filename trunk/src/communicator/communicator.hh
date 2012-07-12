/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <list>
#include <map>
#include "../protocol/message.hh"
#include "connection.hh"
#include "../common/enums.hh"

using namespace std;

/**
 * Abstract Communication module for all components.
 * Handles I/O Multiplexing
 */

class Communicator {
public:

	/**
	 * Constructor
	 */

	Communicator(); // constructor

	/**
	 * Destructor
	 */

	virtual ~Communicator(); // destructor

//	void waitForMessage();

	/**
	 * Establish a connection to a component. Save the connection to list
	 * @param ip Destination IP
	 * @param port Destination Port
	 * @param Destination type: MDS/CLIENT/MONITOR/OSD
	 */

	void addConnection(string ip, uint16_t port, ComponentType connectionType); // establish a connection

	/**
	 * Disconnect from component and remove from list
	 * @param sockfd
	 */

	void removeConnection(uint32_t sockfd); // kill and remove connection

	/**
	 * When there are multiple MDS, choose one
	 * @return Socket descriptor of chosen MDS
	 */

	uint32_t getMdsSockfd();

	/**
	 * When there are multiple monitor, choose one
	 * @return Socket descriptor of chosen Monitor
	 */

	uint32_t getMonitorSockfd();

private:
	map<uint32_t, Connection> _mdsConnectionMap;
	map<uint32_t, Connection> _osdConnectionMap;
	map<uint32_t, Connection> _monitorConnectionMap;
	map<uint32_t, Connection> _clientConnectionMap;
	list<Message *> _outMessageList; // queue of message to be sent
};
#endif
