#ifndef __GET_SEGMENT_CODING_INFO_REPLY_HH__
#define __GET_SEGMENT_CODING_INFO_REPLY_HH__

#include "../message.hh"
#include "../common/segmentcodinginfo.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class GetSegmentCodingInfoReplyMsg: public Message {
public:

	GetSegmentCodingInfoReplyMsg(Communicator* communicator);

	GetSegmentCodingInfoReplyMsg(Communicator* communicator, uint32_t osdSockfd,
	        vector<SegmentCodingInfo>);

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
	vector<SegmentCodingInfo> _segmentCodingInfo;
};

#endif
