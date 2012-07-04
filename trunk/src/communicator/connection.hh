#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include "../common/enums.hh"

#include <stdint.h>

class Connection {
public:
	Connection ();
	void connect (uint32_t ip, uint16_t port);
	void disconnect ();
	void waitForConnection ();
	bool checkTimeout(u32int_t timeoutPeriod);

	uint32_t send (Message msg);
	Message recv ();
private:
	uint32_t _ip;
	uint16_t _port;
	uint32_t _sockfd;
	uint32_t _timestamp;
	ComponentType _connectionType;
};

#endif
