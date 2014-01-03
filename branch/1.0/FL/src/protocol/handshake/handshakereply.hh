#ifndef __HANDSHAKEREPLY_HH__
#define __HANDSHAKEREPLY_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class HandshakeReplyMsg: public Message {
public:

	HandshakeReplyMsg(Communicator* communicator);

	HandshakeReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t srcSockfd,
			uint32_t componentId, ComponentType componentType);

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
	uint32_t _componentId;
	ComponentType _componentType;
};

#endif
