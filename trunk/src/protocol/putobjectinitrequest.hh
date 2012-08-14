#ifndef __PUTOBJECTINITREQUEST_HH__
#define __PUTOBJECTINITREQUEST_HH__

#include "../common/enums.hh"
#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class PutObjectInitRequestMsg: public Message {
public:

	PutObjectInitRequestMsg(Communicator* communicator);

	PutObjectInitRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t objectId, uint32_t objectSize, uint32_t chunkCount,
			CodingScheme codingScheme);

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
	uint64_t _objectSize;
	uint32_t _chunkCount;
	CodingScheme _codingScheme;
};

#endif
