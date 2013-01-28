#ifndef __PRECACHE_SEGMENT_REQUEST_HH__
#define __PRECACHE_SEGMENT_REQUEST_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class PrecacheSegmentRequestMsg: public Message {
public:

	PrecacheSegmentRequestMsg(Communicator* communicator);

	PrecacheSegmentRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t clientId, uint64_t segmentId);

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
	uint32_t _clientId;
	uint64_t _segmentId;
};

#endif
