/*
 * messagefactory.cc
 */

#include "../common/debug.hh"
#include "../common/enums.hh"
#include "message.hh"
#include "listdirectoryrequest.hh"
#include "listdirectoryreply.hh"
#include "putobjectinitrequest.hh"
#include "putobjectinitreply.hh"
#include "putobjectendrequest.hh"
#include "putobjectendreply.hh"
#include "putsegmentinitrequest.hh"
#include "objectdatamsg.hh"
#include "segmentdatamsg.hh"
#include "messagefactory.hh"
#include "handshakerequest.hh"
#include "handshakereply.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(Communicator* communicator,
		MsgType messageType) {
	switch (messageType) {
	case (HANDSHAKE_REQUEST):
		return new HandshakeRequestMsg(communicator);
		break;
	case (HANDSHAKE_REPLY):
		return new HandshakeReplyMsg(communicator);
		break;
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
	case (PUT_OBJECT_END_REQUEST):
		return new PutObjectEndRequestMsg(communicator);
		break;
	case (PUT_OBJECT_END_REPLY):
		return new PutObjectEndReplyMsg(communicator);
		break;
	case (PUT_SEGMENT_INIT_REQUEST):
		return new PutSegmentInitRequestMsg(communicator);
		break;
	case (PUT_SEGMENT_INIT_REPLY):
		// return new PutSegmentInitReplyMsg(communicator);
		return NULL; // to be implemented
		break;
	case (PUT_SEGMENT_END_REQUEST):
		// return new PutSegmentEndRequestMsg(communicator);
		return NULL; // to be implemented
		break;
	case (PUT_SEGMENT_END_REPLY):
		// return new PutSegmentEndReplyMsg(communicator);
		return NULL; // to be implemented
		break;
	case (OBJECT_DATA):
		return new ObjectDataMsg(communicator);
		break;
	case (SEGMENT_DATA):
		// return new SegmentDataMsg(communicator);
		return NULL; // to be implemented
		break;

	default:
		debug("%s\n", "Invalid message type");
		break;
	}
	return NULL;
}
