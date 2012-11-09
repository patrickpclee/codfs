#ifndef __RECOVERY_TRIGGER_REQUEST_HH__
#define __RECOVERY_TRIGGER_REQUEST_HH__

#include <vector>
#include <string>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class RecoveryTriggerRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	RecoveryTriggerRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Objects
	 * @param	mdsSockfd	Socket descriptor
	 */

	RecoveryTriggerRequestMsg (Communicator* communicator, uint32_t osdSockfd, const vector<uint32_t> &osdList);

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
	vector<uint32_t> _osdList;
};

#endif
