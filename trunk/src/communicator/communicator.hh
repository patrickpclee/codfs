/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <list>
#include <map>
#include <atomic>
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
	 * Add message to _outMessageQueue
	 * Set a unique request ID if request ID = 0
	 * @param message Message to send
	 */

	void addMessage(Message* message);

	/**
	 * Check the Message queue, when there is Message pending, dequeue and send
	 */


	void sendMessage();

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
	 * @return Socket descriptor of chosen MDS, -1 if no MDS found
	 */

	uint32_t getMdsSockfd();

	/**
	 * When there are multiple monitor, choose one
	 * @return Socket descriptor of chosen Monitor -1 if no monitor found
	 */

	uint32_t getMonitorSockfd();

	/**
	 * Generate a monotonically increasing requestID
	 * @return Generated requestID
	 */

	uint32_t generateRequestId ();

private:

	/**
	 * Runs in separate detached thread
	 * Execute message->parse function
	 * @param message
	 */

	static void handleThread(Message* message);

	/**
	 * Get the MsgType from raw buffer and get a Message object from the MessageFactory
	 * Execute message.handle() in a separate thread
	 * @param buf Pointer to the buffer holding the Message
	 * @param sockfd Socket Descriptor of incoming connection
	 */

	void dispatch(char* buf, uint32_t sockfd);

	atomic<uint32_t> _requestId;
	map<uint32_t, Connection> _connectionMap;
	list<Message *> _outMessageQueue; // queue of message to be sent
};
#endif
