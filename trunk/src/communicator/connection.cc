#include <iostream>
#include <sys/socket.h>		// for socket() and connect()
#include <sys/types.h>			// for connect()
#include <netinet/in.h>		// for struct sockaddr_in
#include <arpa/inet.h>			// for ip conversion
#include <netdb.h>				// for gethostbyname()
#include <string.h>
#include <stdio.h>
#include "connection.hh"
#include "../common/enums.hh"

using namespace std;

/**
 * Connection Constructor, connect immediately
 * @param ip Destination IP
 * @param port Destination Port
 * @param connectionType Destination Type
 */

Connection::Connection(string ip, uint16_t port, ComponentType connectionType) {
	doConnect(ip, port, connectionType);
}

/**
 * Establish connection with a component
 * @param ip Destination IP
 * @param port Destination Port
 * @param connectionType Destination Type
 */

void Connection::doConnect(string ip, uint16_t port,
		ComponentType connectionType) {
	struct sockaddr_in servaddr; 	// connection struct
	uint32_t sockfd; 				// resulting sockfd
	uint32_t co;					// connection result
	struct hostent *ht; 			// lookup result

	printf("Connecting to %s:%d (Type: %d)\n", ip.c_str(), port,
			connectionType);

	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// fill up destination info in servaddr
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	ht = gethostbyname(ip.c_str());

	if (ht == NULL) {
		cerr << "ERROR: invalid host";
		return;
	}

	memcpy(&servaddr.sin_addr, ht->h_addr, ht->h_length);

	// establish connection
	co = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (co < 0) {
		cerr << "ERROR: connect failed";
		return;
	}

	printf("Connected to sockfd = %d\n", sockfd);

	// save connection info in private variables
	_sockfd = sockfd;
	_ip = ip;
	_port = port;
	_connectionType = connectionType;
}

/**
 * Get back socket descriptor
 * @return socket descriptor of this connection
 */

uint32_t Connection::getSockfd() {
	return _sockfd;
}

/**
 * Disconnect from component
 */

void Connection::disconnect() {
	close(_sockfd);
}
