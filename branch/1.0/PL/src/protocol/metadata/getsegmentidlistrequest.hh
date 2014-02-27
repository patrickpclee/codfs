#ifndef __GET_SEGMENT_ID_LIST_REQUEST_HH__
#define __GET_SEGMENT_ID_LIST_REQUEST_HH__

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

class GetSegmentIdListRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetSegmentIdListRequestMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 */
	GetSegmentIdListRequestMsg(Communicator* communicator, uint32_t mdsSockfd, uint32_t clientId, uint32_t numOfObjs);

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

	void setNumOfObjs(uint32_t numOfObjs);
	uint32_t getNumOfObjs();

	void setSegmentIdList (vector<uint64_t> segmentIdList);
	vector<uint64_t> getSegmentIdList ();

	void setPrimaryList (vector<uint32_t> primaryList);
	vector<uint32_t> getPrimaryList ();

private:
	uint32_t _clientId;
	uint32_t _numOfObjs;
	vector<uint64_t> _segmentIdList;
	vector<uint32_t> _primaryList;
};

#endif
