/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <atomic>
#include "socket.hh"
#include "../protocol/messagefactory.hh"
#include "../protocol/message.hh"
#include "connection.hh"
#include "../common/enums.hh"
#include "component.hh"

using namespace std;

// forward declaration to avoid circular dependency
class Message;
class MessageFactory;
class Connection;

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
	 * @param expectReply Whether or not this message should wait for a reply
	 */

	void addMessage(Message* message, bool expectReply = false);

	/**
	 * Check the Message queue, when there is Message pending, dequeue and send
	 */

	void sendMessage();

	/**
	 * Create a server socket at the specified port
	 * @return Binded Socket
	 */

	void createServerSocket();

	/**
	 * Establish a connection to a component. Save the connection to list
	 * @param ip Destination IP
	 * @param port Destination Port
	 * @param Destination type: MDS/CLIENT/MONITOR/OSD
	 * @return Sockfd Descriptor of the new connection
	 */

	uint32_t connectAndAdd(string ip, uint16_t port,
			ComponentType connectionType); // establish a connection

	/**
	 * Disconnect from component and remove from list
	 * @param sockfd
	 */

	void disconnectAndRemove(uint32_t sockfd); // kill and remove connection

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

	// FOR TESTING ONLY
	uint32_t getOsdSockfd();

	/**
	 * Generate a monotonically increasing requestID
	 * @return Generated requestID
	 */

	uint32_t generateRequestId();

	/**
	 * Retrieve the pointer to a sent message by its requestId
	 * Remove the message from sentMessageQueue
	 * @param requestId Request ID
	 * @return Pointer to sent message
	 */

	Message* popWaitReplyMessage(uint32_t requestId);

	/**
	 * Connect to all components specified in the config file
	 */

	void connectAllComponents();

	/**
	 * Runs in a separate detached thread
	 * Execute communicator->sendMessage function
	 * @param communicator Corresponding communicator of the component
	 */
	static void sendThread(Communicator* communicator);

	/**
	 * Set the Component ID
	 * @param id Component ID
	 */

	void setId(uint32_t id);

	/**
	 * Set the Component Type
	 * @param componentType Component Type
	 */

	void setComponentType(ComponentType componentType);

	void handshakeRequestProcessor(uint32_t requestId, uint32_t _sockfd,
			uint32_t componentId, ComponentType componentType);

protected:

	/**
	 * Runs in a separate detached thread
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

	/**
	 * Check if out message queue is empty
	 * @return True if empty, false otherwise
	 */

	bool isOutMessageQueueEmpty();

	/**
	 * Delete the message when it is deletable
	 * @param message Message pointer to delete
	 */

	void waitAndDelete(Message* message);

	/**
	 * Connect to all the component in the list
	 * @param componentList Component List
	 */

	void connectToComponents(vector<Component> componentList);

	/**
	 * Parse the config file and extract information about components
	 * @param componentType Type of components to extract
	 * @return List of the component information
	 */

	vector<struct Component> parseConfigFile(string componentType);

	/**
	 * Send a handshake request to the destination component
	 * @param sockfd My Socket Descriptor
	 * @param componentId My Component ID
	 * @param componentType My Component Type
	 */

	void sendHandshakeRequest(uint32_t sockfd, uint32_t componentId,
			ComponentType componentType);


	/**
	 * DEBUG: Print the component information saved in the list
	 * @param componentType Type of component to print (for heading)
	 * @param componentList List of component information
	 */

	void printComponents(string componentType, vector<Component> componentList);

	atomic<uint32_t> _requestId; // atomic monotically increasing request ID

	uint16_t _serverPort; // listening port for incoming connections
	Socket _serverSocket; // socket for accepting incoming connections
	map<uint32_t, Connection*> _connectionMap; // a map of all connections
	map<uint32_t, uint32_t> _componentIdMap; // a map from component ID to sockfd
	list<Message *> _outMessageQueue; // queue of message to be sent
	map<uint32_t, Message *> _waitReplyMessageMap; // map of message waiting for reply
	uint32_t _maxFd; // maximum number of socket descriptors among connections

	// self identity
	ComponentType _componentType;
	uint32_t _id; // id of myself

	// config values
	uint32_t _timeoutSec, _timeoutUsec;
	uint32_t _chunkSize;
};
#endif
