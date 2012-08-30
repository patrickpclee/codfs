#ifndef __GET_OBJECT_ID_LIST_REPLY_HH__
#define __GET_OBJECT_ID_LIST_REPLY_HH__

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

class GetObjectIdListReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetObjectIdListReplyMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 */
	GetObjectIdListReplyMsg(Communicator* communicator, uint32_t mdsSockfd,
			vector<uint64_t>objectIdList);

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

	void setObjectIdList (vector<uint64_t> objectIdList);
	vector<uint64_t> getObjectIdList ();

private:
	vector<uint64_t> _objectIdList;
};

#endif
