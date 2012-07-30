/*
 * messagefactory.cc
 */

#include "../common/debug.hh"
#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/listdirectoryreply.hh"
#include "../protocol/putobjectinit.hh"
#include "../protocol/objectdatamsg.hh"
#include "../protocol/putobjectend.hh"
#include "messagefactory.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(Communicator* communicator,
		MsgType messageType) {
	switch (messageType) {
	case (LIST_DIRECTORY_REQUEST):
		return new ListDirectoryRequestMsg(communicator);
		break;
	case (LIST_DIRECTORY_REPLY):
		return new ListDirectoryReplyMsg(communicator);
		break;
	case (PUT_OBJECT_INIT):
		return new PutObjectInitMsg(communicator);
		break;
	case (OBJECT_DATA):
		return new ObjectDataMsg(communicator);
		break;
	case (PUT_OBJECT_END):
		return new PutObjectEndMsg(communicator);
		break;

	default:
		debug("%s\n", "Invalid message type");
		break;
	}
	return NULL;
}
