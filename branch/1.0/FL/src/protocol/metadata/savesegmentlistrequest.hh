#ifndef __SAVE_SEGMENT_LIST_REQUEST_HH__
#define __SAVE_SEGMENT_LIST_REQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 */

class SaveSegmentListRequestMsg: public Message {
public:

	SaveSegmentListRequestMsg(Communicator* communicator);

	SaveSegmentListRequestMsg(Communicator* communicator, uint32_t sockfd, uint32_t clientId, uint32_t fileId, const vector<uint64_t> &segmentList);

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
	uint32_t _clientId;
	uint32_t _fileId;
	vector<uint64_t> _segmentList;
};

#endif
