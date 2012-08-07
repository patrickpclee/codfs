#ifndef __PUTSEGMENTINITREPLY_HH__
#define __PUTSEGMENTINITREPLY_HH__

#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class PutSegmentInitReplyMsg: public Message {
public:

	PutSegmentInitReplyMsg(Communicator* communicator);

	PutSegmentInitReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t osdSockfd,
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

	void handle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	uint64_t _objectId;
	uint32_t _segmentId;
};

#endif
