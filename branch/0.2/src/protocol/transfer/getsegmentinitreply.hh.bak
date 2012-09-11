#ifndef __GETSEGMENTINITREPLY_HH__
#define __GETSEGMENTINITREPLY_HH__

#include "../message.hh"

using namespace std;

class GetSegmentInitReplyMsg: public Message {
public:

	GetSegmentInitReplyMsg(Communicator* communicator);

	GetSegmentInitReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId, uint32_t segmentSize, uint32_t chunkCount);

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
	uint32_t _segmentSize;
	uint32_t _chunkCount;
};

#endif
