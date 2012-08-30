#ifndef __GET_OBJECT_ID_LIST_REQUEST_HH__
#define __GET_OBJECT_ID_LIST_REQUEST_HH__

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

class GetObjectIdListRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetObjectIdListRequestMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 */
	GetObjectIdListRequestMsg(Communicator* communicator, uint32_t mdsSockfd,
			uint32_t numOfObjs);

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

	void setObjectIdList (vector<uint64_t> objectIdList);
	vector<uint64_t> getObjectIdList ();

private:
	uint32_t _numOfObjs;
	vector<uint64_t> _objectIdList;
};

#endif
