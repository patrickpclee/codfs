#ifndef __BLOCK_TRANSFER_END_REQUEST_HH__
#define __BLOCK_TRANSFER_END_REQUEST_HH__

#include "../message.hh"
#include "../common/blocklocation.hh"

using namespace std;

/**
 * Extends the Message class
 */

class BlockTransferEndRequestMsg: public Message {
public:

	BlockTransferEndRequestMsg(Communicator* communicator);

	BlockTransferEndRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t segmentId, uint32_t blockId, DataMsgType dataMsgType,
			string updateKey, vector<offset_length_t> offsetLength, vector<BlockLocation> parityList);

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
	DataMsgType _dataMsgType;
	string _updateKey;
	vector<offset_length_t> _offsetLength;
	vector<BlockLocation> _parityList;
};

#endif
