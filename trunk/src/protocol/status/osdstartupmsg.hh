#ifndef __OSDSTARTUPMSG_HH__
#define __OSDSTARTUPMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class OsdStartupMsg: public Message {
public:

	OsdStartupMsg(Communicator* communicator);

	OsdStartupMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t osdId, uint32_t capacity, uint32_t loading);
	OsdStartupMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t osdId, uint32_t capacity, uint32_t loading, uint32_t ip,
			uint16_t port);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse(char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	uint32_t _osdId;
	uint32_t _capacity;
	uint32_t _loading;
	uint32_t _osdIp;
	uint16_t _osdPort;
};

#endif
