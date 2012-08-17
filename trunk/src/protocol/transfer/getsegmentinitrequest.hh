#ifndef __GETSEGMENTINITREQUEST_HH__
#define __GETSEGMENTINITREQUEST_HH__

#include "../message.hh"

using namespace std;

class GetSegmentInitRequestMsg: public Message {
public:

	GetSegmentInitRequestMsg(Communicator* communicator);

	GetSegmentInitRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t objectId, uint32_t segmentId);

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

	/*
	void setSegmentSize(uint32_t segmentSize);
	uint32_t getSegmentSize();
	void setChunkCount(uint32_t chunkCount);
	uint32_t getChunkCount();
	*/

private:
	uint64_t _objectId;
	uint32_t _segmentId;
};

#endif
