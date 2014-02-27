#ifndef __GET_SEGMENT_INFO_REPLY_HH__
#define __GET_SEGMENT_INFO_REPLY_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class GetSegmentInfoReplyMsg: public Message {
public:

	GetSegmentInfoReplyMsg(Communicator* communicator);

	GetSegmentInfoReplyMsg(Communicator* communicator, uint32_t requestId,
			uint32_t dstSockfd, uint64_t segmentId, uint32_t segmentSize, const vector<uint32_t> &nodeList,
			CodingScheme codingScheme, const string &codingSetting);

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
	uint32_t _segmentSize;

	vector<uint32_t> _nodeList;
	CodingScheme _codingScheme;
	string _codingSetting;
};

#endif
