#ifndef __SEGMENTDATAMSG_HH__
#define __SEGMENTDATAMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class SegmentDataMsg: public Message {
public:

	SegmentDataMsg(Communicator* communicator);

	SegmentDataMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t segmentId, uint64_t offset, uint32_t length, DataMsgType dataMsgType, string updateKey);

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
	uint64_t _offset;
	uint32_t _length;
	DataMsgType _dataMsgType;
	string _updateKey;
};

#endif
