#include <iostream>
using namespace std;

#include "recoverytriggerreply.hh"
#include "recoverytriggerrequest.hh"
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
		ncvfs::ObjectLocationPro* olp =
			recoveryTriggerReplyPro.add_objectlocations();
		olp->set_objectid (ol.objectId);
		olp->set_primaryid (ol.primaryId);
		for (uint32_t osdId: ol.osdList) {
			olp->add_osdlist (osdId);
		}
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

	_objectLocations.clear();
	for (int i = 0; i < recoveryTriggerReplyPro.objectlocations_size(); i++) {
		ncvfs::ObjectLocationPro olp = recoveryTriggerReplyPro.objectlocations(i);
		struct ObjectLocation ol;
		ol.objectId = olp.objectid();
		ol.primaryId = olp.primaryid();
		ol.osdList.clear();
		for (int j = 0; j < olp.osdlist_size(); j++)
			ol.osdList.push_back(olp.osdlist(j));
		_objectLocations.push_back(ol);
	}

	return;
}

void RecoveryTriggerReplyMsg::doHandle() {
	RecoveryTriggerRequestMsg* recoveryTriggerRequestMsg =
			(RecoveryTriggerRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	recoveryTriggerRequestMsg->setObjectLocations(_objectLocations);
	recoveryTriggerRequestMsg->setStatus(READY);
}

void RecoveryTriggerReplyMsg::printProtocol() {
	debug("%s\n", "[RECOVERY_TRIGGER_REPLY] GOT.");
}
