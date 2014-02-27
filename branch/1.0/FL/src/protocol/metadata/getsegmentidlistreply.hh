#ifndef __GET_SEGMENT_ID_LIST_REPLY_HH__
#define __GET_SEGMENT_ID_LIST_REPLY_HH__

#include <string>
#include <vector>
#include <future>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Requset to upload file
 */

class GetSegmentIdListReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetSegmentIdListReplyMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 */
	GetSegmentIdListReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t mdsSockfd, const vector<uint64_t> &segmentIdList, const vector<uint32_t> &primaryList);

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

	//void setSegmentIdList (vector<uint64_t> segmentIdList);
	//vector<uint64_t> getSegmentIdList ();

private:
	vector<uint64_t> _segmentIdList;
	vector<uint32_t> _primaryList;
};

#endif
