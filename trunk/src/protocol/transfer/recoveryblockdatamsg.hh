#ifndef __RECOVERYBLOCKDATAMSG_HH__
#define __RECOVERYBLOCKDATAMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an block transfer
 */

class RecoveryBlockDataMsg: public Message {
public:

	RecoveryBlockDataMsg(Communicator* communicator);

	/**
	 *
	 * @param communicator
	 * @param dstSockfd
	 * @param segmentId
	 * @param blockId
	 * @param length
	 */

	RecoveryBlockDataMsg(Communicator* communicator, uint32_t requestId,
			uint32_t dstSockfd, uint64_t segmentId, uint32_t blockId,
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
	uint64_t _segmentId;
	uint32_t _blockId;
	uint32_t _length;
};

#endif
