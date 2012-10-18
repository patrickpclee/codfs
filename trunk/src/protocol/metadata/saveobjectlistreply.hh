#ifndef __SAVE_OBJECT_LIST_REPLY_HH__
#define __SAVE_OBJECT_LIST_REPLY_HH__

//#include <future>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

class SaveObjectListReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	SaveObjectListReplyMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	mdsSockfd	Socket descriptor of MDS
	 */
	SaveObjectListReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg ();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse (char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle ();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol ();

private:
	uint32_t _fileId;
};

#endif
