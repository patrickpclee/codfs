/*
 * messagefactory.cc
 */

#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "messagefactory.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(MsgType messageType) {
	Message* message;
	switch (messageType) {
	case (LIST_DIRECTORY_REQUEST):
		return new ListDirectoryRequestMsg();
		break;
	case (LIST_DIRECTORY_REPLY):
		return new ListDirectoryReplyMsg();
		break;
	}
	return NULL;
}
