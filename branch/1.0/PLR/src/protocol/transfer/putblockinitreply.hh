#ifndef __PUTBLOCKINITREPLY_HH__
#define __PUTBLOCKINITREPLY_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class PutBlockInitReplyMsg: public Message {
public:

	PutBlockInitReplyMsg(Communicator* communicator);

	PutBlockInitReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t osdSockfd,
			uint64_t segmentId, uint32_t blockId);

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
	uint32_t _blockId;
};

#endif
