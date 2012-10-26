#ifndef __SWITCHPRIMARYOSDREQUESTMSG_HH__
#define __SWITCHPRIMARYOSDREQUESTMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class SwitchPrimaryOsdRequestMsg: public Message {
public:

	SwitchPrimaryOsdRequestMsg(Communicator* communicator);

	SwitchPrimaryOsdRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			uint32_t clientId, uint64_t objectId);

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

	uint32_t getNewPrimaryOsdId();

	void setNewPrimaryOsdId(uint32_t osdId);

private:
	uint32_t _clientId;
	uint64_t _objectId;
	uint32_t _newPrimaryOsdId;
};

#endif
