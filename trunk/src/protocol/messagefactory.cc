/*
 * messagefactory.cc
 */

#include "../common/debug.hh"
#include "../common/enums.hh"
#include "../protocol/message.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/listdirectoryreply.hh"
#include "../protocol/putobjectinitrequest.hh"
#include "../protocol/putobjectinitreply.hh"
#include "../protocol/objectdatamsg.hh"
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
	case (PUT_OBJECT_INIT_REQUEST):
		return new PutObjectInitRequestMsg(communicator);
		break;
	case (PUT_OBJECT_INIT_REPLY):
		return new PutObjectInitReplyMsg(communicator);
		break;
	case (OBJECT_DATA):
		return new ObjectDataMsg(communicator);
		break;

	default:
		debug("%s\n", "Invalid message type");
		break;
	}
	return NULL;
}
