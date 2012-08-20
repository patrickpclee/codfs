#ifndef __GETOBJECTREPLY_HH__
#define __GETOBJECTREPLY_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class GetObjectReplyMsg: public Message {
public:

	GetObjectReplyMsg(Communicator* communicator);

	GetObjectReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t osdSockfd,
			uint64_t objectId, uint32_t objectSize, uint32_t chunkCount);

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
	uint32_t _objectSize;
	uint32_t _chunkCount;
};

#endif
