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
		uint32_t requestId, uint32_t sockfd, const vector<struct SegmentLocation> &segmentLocations) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_segmentLocations = segmentLocations;

}

void RecoveryTriggerReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryTriggerReplyPro recoveryTriggerReplyPro;

	//TODO push segment location list.
	for (struct SegmentLocation ol: _segmentLocations) {
		ncvfs::SegmentLocationPro* olp =
			recoveryTriggerReplyPro.add_segmentlocations();
		olp->set_segmentid (ol.segmentId);
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

	_segmentLocations.clear();
	for (int i = 0; i < recoveryTriggerReplyPro.segmentlocations_size(); i++) {
		ncvfs::SegmentLocationPro olp = recoveryTriggerReplyPro.segmentlocations(i);
		struct SegmentLocation ol;
		ol.segmentId = olp.segmentid();
		ol.primaryId = olp.primaryid();
		ol.osdList.clear();
		for (int j = 0; j < olp.osdlist_size(); j++)
			ol.osdList.push_back(olp.osdlist(j));
		_segmentLocations.push_back(ol);
	}

	return;
}

void RecoveryTriggerReplyMsg::doHandle() {
	RecoveryTriggerRequestMsg* recoveryTriggerRequestMsg =
			(RecoveryTriggerRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	recoveryTriggerRequestMsg->setSegmentLocations(_segmentLocations);
	recoveryTriggerRequestMsg->setStatus(READY);
}

void RecoveryTriggerReplyMsg::printProtocol() {
	debug("%s\n", "[RECOVERY_TRIGGER_REPLY] GOT.");
}
