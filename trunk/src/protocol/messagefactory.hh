/*
 * messagefactory.cc
 */

#ifndef __MESSAGEFACTORY_HH__
#define __MESSAGEFACTORY_HH__

#include "message.hh"
#include "../common/enums.hh"

class MessageFactory {
public:
	MessageFactory();
	~MessageFactory();
	static Message* createMessage (MsgType messageType);
};

#endif
