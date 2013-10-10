#ifndef __GET_SEGMENT_CODING_INFO_REQUEST_HH__
#define __GET_SEGMENT_CODING_INFO_REQUEST_HH__

#include "../common/segmentcodinginfo.hh"
#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class GetSegmentCodingInfoRequestMsg: public Message {
public:

	GetSegmentCodingInfoRequestMsg(Communicator* communicator);

	GetSegmentCodingInfoRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			vector<uint64_t> segmentId);

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

	void setSegmentCodingInfoList(vector<SegmentCodingInfo> segmentCodingInfoList);
	vector<SegmentCodingInfo> getSegmentCodingInfoList();

private:
	vector<uint64_t> _segmentIdList;
	vector<SegmentCodingInfo> _segmentCodingInfoList;
};

#endif
