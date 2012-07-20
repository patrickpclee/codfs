/*
 * messagefactory.cc
 */

#ifndef __MESSAGEFACTORY_HH__
#define __MESSAGEFACTORY_HH__

#include "message.hh"
#include "../common/enums.hh"

class Message;

class Communicator;

class MessageFactory {
public:
	MessageFactory();
	~MessageFactory();
	static Message* createMessage(Communicator* communicator,
			MsgType messageType);
};

#endif
