#ifndef __CONNECTION_HH__
#define __CONNECTION_HH__

#include <stdint.h>
#include <string>
#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "socket.hh"

using namespace std;

/**
 * Handle connections to components
 */

class Connection {
public:

	/**
	 * Constructor
	 */

	Connection();

	/**
	 * Constructor (connect immediately)
	 * @param ip Destination IP
	 * @param port Destination Port
	 * @param connectionType Destination Type
	 */

	Connection(string ip, uint16_t port, ComponentType connectionType);

	/**
	 * Establish connection with a component
	 * @param ip Destination IP
	 * @param port Destination Port
	 * @param connectionType Destination Type
	 * @return Socket Descriptor
	 */

	uint32_t doConnect(string ip, uint16_t port, ComponentType connectionType);

	/**
	 * Disconnect from component
	 */

	void disconnect();

	/**
	 * Send a message to the connection
	 * @param msg Pointer to message to send
	 * @return Bytes sent
	 */

	uint32_t sendMessage (Message *message);
    uint32_t sendMessages (vector<Message*> messages);

	/**
	 * Receive a message from the connection
	 * @return Pointer to message received
	 */

	char* recvMessage ();

	/**
	 * Retrive the underlying Socket
	 * @return Socket segment of the connection
	 */

	Socket* getSocket();

	/**
	 * Retrive the underlying Socket Descriptor
	 * @return Socket Descriptor of the connection
	 */

	uint32_t getSockfd();

	/**
	 * Set the type of destination component
	 * @param type Component Type
	 */

	void setConnectionType(ComponentType type);

	/**
	 * Retrive the type of destination component
	 * @return Destination component type
	 */

	ComponentType getConnectionType();

	void setIsDisconnected (bool isDisconnected) {
		_isDisconnected = isDisconnected;
	}

	bool getIsDisconnected () {
		return _isDisconnected;
	}


private:
	Socket _socket;
	ComponentType _connectionType;
	bool _isDisconnected;
};

#endif
