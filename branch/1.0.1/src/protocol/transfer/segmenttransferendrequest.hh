#ifndef __SEGMENT_TRANSFER_END_REQUEST_HH__
#define __SEGMENT_TRANSFER_END_REQUEST_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class SegmentTransferEndRequestMsg: public Message {
public:

	SegmentTransferEndRequestMsg(Communicator* communicator);

	SegmentTransferEndRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t segmentId, DataMsgType dataMsgType, string updateKey,
			vector<offset_length_t> offsetlength);

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
	DataMsgType _dataMsgType;
	string _updateKey;
	vector<offset_length_t> _offsetLength;
};

#endif
