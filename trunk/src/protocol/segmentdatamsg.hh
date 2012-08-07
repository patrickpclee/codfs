#ifndef __SEGMENTDATAMSG_HH__
#define __SEGMENTDATAMSG_HH__

#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment transfer
 */

class SegmentDataMsg: public Message {
public:

	SegmentDataMsg(Communicator* communicator);

	/**
	 *
	 * @param communicator
	 * @param dstSockfd
	 * @param objectId
	 * @param segmentId
	 * @param offset
	 * @param length
	 */

	SegmentDataMsg(Communicator* communicator, uint32_t dstSockfd,
			uint64_t objectId,
			uint32_t segmentId,
			uint64_t offset,
			uint32_t length);

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
	uint32_t _segmentId;
	uint64_t _offset;
	uint32_t _length;
};

#endif
