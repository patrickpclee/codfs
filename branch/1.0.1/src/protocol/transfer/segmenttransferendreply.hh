#ifndef __SEGMENT_TRANSFER_END_REPLY_HH__
#define __SEGMENT_TRANSFER_END_REPLY_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class SegmentTransferEndReplyMsg: public Message {
public:

	SegmentTransferEndReplyMsg(Communicator* communicator);

	SegmentTransferEndReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t dstSockfd,
			uint64_t segmentId, bool isSmallSegment = false);

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
	uint64_t _segmentId;
	bool _isSmallSegment;
};

#endif
