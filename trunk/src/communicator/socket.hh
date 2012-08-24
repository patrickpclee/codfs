// Definition of the Socket class

#ifndef __SOCKET_HH__
#define __SOCKET_HH__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 10;

class Socket {
public:

	/**
	 * Constructor
	 */

	Socket();

	/**
	 * Destructor
	 */

	virtual ~Socket();

	// Server initialization

	/**
	 * Create a socket
	 * @return true if success, false if error
	 */

	bool create();

	/**
	 * Bind a socket to a port
	 * @param port Desired port number
	 * @return true if success, false if error
	 */

	bool bind(const int port);

	/**
	 * Set the maximum connections to listen
	 * @return true if success, false if error
	 */

	bool listen() const;

	/**
	 * Accept an incoming connection and save the sockfd in the Socket object
	 * @param Pointer to a new socket object (for the new connection)
	 * @return true if success, false if error
	 */

	bool accept(Socket*) const;

	// Client initialization

	/**
	 * Initialize a connection to the destination
	 * @param host Destination IP
	 * @param port Destination port
	 * @return true if success, false if error
	 */

	bool connect(const std::string host, const int port);

	// Data Transimission

	/**
	 * Send a certain number of bytes from a buffer to the socket
	 * @param buf Buffer to send
	 * @param buf_len Length to send
	 * @return Number of bytes sent
	 */

	int32_t sendn(const char* buf, int32_t buf_len);

	/**
	 * Receive a certain number of bytes form a socket to buffer
	 * @param buf Buffer to store the received data
	 * @param buf_len Lenth to receive
	 * @return Number of bytes received
	 */

	int32_t recvn(char* buf, int32_t buf_len);

	void set_non_blocking(const bool);

	/**
	 * Check if the socket number is valid
	 * @return true if valid, false if invalid
	 */

	bool is_valid() const {
		return m_sock != -1;
	}

	uint32_t getSockfd();

private:

	int m_sock;
	sockaddr_in m_addr;

};

#endif
