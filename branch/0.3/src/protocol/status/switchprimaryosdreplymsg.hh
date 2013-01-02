#ifndef __SWITCHPRIMARYOSDREPLYMSG_HH__
#define __SWITCHPRIMARYOSDREPLYMSG_HH__

#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class SwitchPrimaryOsdReplyMsg: public Message {
public:

	SwitchPrimaryOsdReplyMsg(Communicator* communicator);

	SwitchPrimaryOsdReplyMsg(Communicator* communicator, uint32_t requestId,
		uint32_t dstSockfd,	uint32_t newOsdId);

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
	uint32_t _newPrimaryOsdId;
};

#endif
