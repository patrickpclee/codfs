/*
 * messagefactory.cc
 */

#include "../common/debug.hh"
#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/listdirectoryreply.hh"
#include "messagefactory.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(MsgType messageType) {
	switch (messageType) {
	case (LIST_DIRECTORY_REQUEST):
		return new ListDirectoryRequestMsg();
		break;
	case (LIST_DIRECTORY_REPLY):
		return new ListDirectoryReplyMsg();
		break;
	default:
		debug ("%s\n", "Invalid message type");
		break;
	}
	return NULL;
}
