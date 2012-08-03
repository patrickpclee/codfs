#ifndef __PUTOBJECTENDREQUEST_HH__
#define __PUTOBJECTENDREQUEST_HH__

#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class PutObjectEndRequestMsg: public Message {
public:

	PutObjectEndRequestMsg(Communicator* communicator);

	PutObjectEndRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t objectId);

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

	void handle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	uint64_t _objectId;
};

#endif
