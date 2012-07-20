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
	 * Constructor (for saving incoming connection)
	 * @param socket Socket of incoming connection
	 */

//	Connection(Socket socket);

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

	/**
	 * Receive a message from the connection
	 * @return Pointer to message received
	 */

	char* recvMessage ();

	Socket* getSocket();
	int getSockfd();
	ComponentType getConnectionType();


private:
	Socket _socket;
//	uint32_t _sockfd;
//	string _ip;
//	uint16_t _port;
	ComponentType _connectionType;
};

#endif
