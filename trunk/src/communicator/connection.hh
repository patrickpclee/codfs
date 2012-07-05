#ifndef __CONNECTION_HH__
#define __CONNECTION_HH__

#include <string>
#include "../common/enums.hh"

#include <stdint.h>

using namespace std;

class Connection {
public:
	Connection ();
	Connection (string ip, uint16_t port, ComponentType connectionType);
	void doConnect (string ip, uint16_t port, ComponentType connectionType);
	uint32_t getSockfd();
//	void disconnect ();
//	void waitForConnection ();
//	bool checkTimeout(u32int_t timeoutPeriod);

//	uint32_t send (Message msg);
//	Message recv ();
private:
	uint32_t _sockfd;
	string _ip;
	uint16_t _port;
	uint32_t _timestamp;
	ComponentType _connectionType;
};

#endif
