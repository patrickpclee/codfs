#include <iostream>
using namespace std;

#include "recoverytriggerreply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

RecoveryTriggerReplyMsg::RecoveryTriggerReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

RecoveryTriggerReplyMsg::RecoveryTriggerReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, const vector<struct ObjectLocation> &objectLocations) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_objectLocations = objectLocations;

}

void RecoveryTriggerReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryTriggerReplyPro recoveryTriggerReplyPro;

	//TODO push object location list.
	for (struct ObjectLocation ol: _objectLocations) {
		ncvfs::ObjectLocationPro olp;
		olp.set_objectid (ol.objectId);
		olp.set_primaryId (ol.primaryId);

	}

	if (!recoveryTriggerReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(RECOVERY_TRIGGER_REPLY);
	setProtocolMsg(serializedString);

}

void RecoveryTriggerReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RecoveryTriggerReplyPro recoveryTriggerReplyPro;
	recoveryTriggerReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	//TODO parse object location list.

	return;
}

void RecoveryTriggerReplyMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	//monitor->recoverTriggerProcessor (_msgHeader.requestId, _sockfd, _objectLocation);
#endif
}

void RecoveryTriggerReplyMsg::printProtocol() {
	debug("%s\n", "[RECOVERY_TRIGGER_REPLY] GOT.");
}
