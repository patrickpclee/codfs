#ifndef __OBJECTDATAMSG_HH__
#define __OBJECTDATAMSG_HH__

#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class ObjectDataMsg: public Message {
public:

	ObjectDataMsg(Communicator* communicator);

	ObjectDataMsg(Communicator* communicator, uint32_t dstSockfd,
			uint64_t objectId, uint64_t offset, uint32_t length);

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
	uint64_t _objectId;
	uint64_t _offset;
	uint32_t _length;
};

#endif
