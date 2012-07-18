/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <list>
#include <map>
#include "../protocol/messagefactory.hh"
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

	/**
	 * Listen to all the socket descriptors and call select for I/O multiplexing
	 * When a Message is received, call dispatch() to execute handler
	 */

	void waitForMessage();

	/**
	 * Check the Message queue, when there is Message pending, dequeue and send
	 */

	void sendMessage();

	/**
	 * Abstract function
	 * Aanalyze the MsgHeader and create the corresponding Message class
	 * Execute message.handle() in a separate thread
	 * @param buf Pointer to the buffer holding the Message
	 * @param sockfd Socket Descriptor of incoming connection
	 */

	virtual void dispatch(char* buf, uint32_t sockfd) = 0;

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

	/**
	 *
	 */

	void setMessageFactory (MessageFactory* messageFactory);

private:
	MessageFactory* _messageFactory;
	map<uint32_t, Connection> _connectionMap;
	list<Message *> _outMessageQueue; // queue of message to be sent
};
#endif
