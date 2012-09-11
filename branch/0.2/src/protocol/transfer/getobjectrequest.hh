#ifndef __GET_OBJECT_REQUEST_HH__
#define __GET_OBJECT_REQUEST_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class GetObjectRequestMsg: public Message {
public:

	GetObjectRequestMsg(Communicator* communicator);

	GetObjectRequestMsg(Communicator* communicator, uint32_t dstSockfd,
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

	void doHandle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();
	void setObjectSize(uint32_t objectSize);
	void setChunkCount(uint32_t chunkCount);
	uint32_t getObjectSize();
	uint32_t getChunkCount();
	uint32_t getRequestId();

private:
	uint64_t _objectId;
	uint32_t _objectSize;
	uint32_t _chunkCount;
};

#endif
