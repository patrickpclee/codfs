/*
 * messagefactory.cc
 */

#ifndef __MESSAGEFACTORY_HH__
#define __MESSAGEFACTORY_HH__

#include "../common/enums.hh"

class MessageFactory {
	MessageFactory();
	~MessageFactory();
	static Message* createMessage (MsgType messageType);
};

#endif
