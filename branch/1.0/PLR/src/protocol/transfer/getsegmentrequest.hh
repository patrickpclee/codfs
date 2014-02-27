#ifndef __GET_SEGMENT_REQUEST_HH__
#define __GET_SEGMENT_REQUEST_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class GetSegmentRequestMsg: public Message {
public:

	GetSegmentRequestMsg(Communicator* communicator);

	GetSegmentRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			uint64_t segmentId);

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
	uint32_t getRequestId();

private:
	uint64_t _segmentId;
};

#endif
