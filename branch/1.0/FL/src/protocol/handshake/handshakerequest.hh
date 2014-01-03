#ifndef __HANDSHAKEREQUEST_HH__
#define __HANDSHAKEREQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class HandshakeRequestMsg: public Message {
public:

	HandshakeRequestMsg(Communicator* communicator);

	HandshakeRequestMsg(Communicator* communicator, uint32_t srcSockfd,
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

	/**
	 * Set the Component ID replied
	 * @param componentId Component ID
	 */

	void setTargetComponentId(uint32_t componentId);

	/**
	 * Set the component type replied
	 * @param componentType Component Type
	 */

	void setTargetComponentType(ComponentType componentType);

	/**
	 * Get the target component ID
	 * @return Target Component ID
	 */

	uint32_t getTargetComponentId();

	/**
	 * Get the target Component Type
	 * @return Target Component Type
	 */

	ComponentType getTargetComponentType();


private:
	uint32_t _componentId;
	ComponentType _componentType;

	uint32_t _targetComponentId;
	ComponentType _targetComponentType;
};

#endif
