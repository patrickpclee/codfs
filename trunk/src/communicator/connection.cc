#include <stdio.h>
#include <sys/socket.h>		// for socket() and connect()
#include <sys/types.h>			// for connect()
#include <netinet/in.h>		// for struct sockaddr_in
#include <arpa/inet.h>			// for ip conversion
#include <netdb.h>				// for gethostbyname()
#include <string.h>
#include "connection.hh"

using namespace std;

Connection::Connection(string ip, uint16_t port, ComponentType connectionType) {
	doConnect(ip, port, connectionType);
}

void Connection::doConnect(string ip, uint16_t port,
		ComponentType connectionType) {
	struct sockaddr_in servaddr;
	uint32_t sockfd;
	uint32_t co;
	struct hostent *ht;

	printf("Connecting to %s:%d (Type: %d)\n", ip.c_str(), port, connectionType);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// fill up destination info in servaddr
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	ht = gethostbyname(ip.c_str());

	if (ht == NULL) {
		perror("ERROR: invalid host.\n");
		return;
	}

	memcpy(&servaddr.sin_addr, ht->h_addr, ht->h_length);

	co = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (co < 0) {
		perror("ERROR: connect failed.\n");
		return;
	}

	printf("Connected sockfd = %d\n", sockfd);

	_sockfd = sockfd;
	_ip = ip;
	_port = port;
	_connectionType = connectionType;
}

uint32_t Connection::getSockfd() {
	return _sockfd;
}

