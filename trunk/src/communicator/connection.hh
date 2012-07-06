#ifndef __CONNECTION_HH__
#define __CONNECTION_HH__

#include <string>
#include "../common/enums.hh"

#include <stdint.h>

using namespace std;

/**
 * Handle connections to components
 */

class Connection {
public:
	Connection ();
	Connection (string ip, uint16_t port, ComponentType connectionType);
	void doConnect (string ip, uint16_t port, ComponentType connectionType);
	void disconnect ();

//	uint32_t send (Message msg);
//	Message recv ();

	// getters
	uint32_t getSockfd();

private:
	uint32_t _sockfd;
	string _ip;
	uint16_t _port;
	ComponentType _connectionType;
};

#endif
