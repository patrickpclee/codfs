#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "recoverytriggerrequest.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif


RecoveryTriggerRequestMsg::RecoveryTriggerRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

RecoveryTriggerRequestMsg::RecoveryTriggerRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, const vector<uint32_t> &osdList, bool dstSpecified,
		const vector<uint32_t> &dstSpec) :
		Message(communicator) {

	_sockfd = mdsSockfd;
	_osdList = osdList;
	_dstSpecified = dstSpecified;
	_dstSpecOsdList = dstSpec;
}

void RecoveryTriggerRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryTriggerRequestPro recoveryTriggerRequestPro;
	for (uint32_t osd : _osdList) {
		recoveryTriggerRequestPro.add_osdlist(osd);
	}
	recoveryTriggerRequestPro.set_dstspecified(_dstSpecified);
	for (uint32_t osd : _dstSpecOsdList) {
		recoveryTriggerRequestPro.add_dstosdlist(osd);
	}

	if (!recoveryTriggerRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (RECOVERY_TRIGGER_REQUEST);
	setProtocolMsg(serializedString);

}

void RecoveryTriggerRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RecoveryTriggerRequestPro recoveryTriggerRequestPro;
	recoveryTriggerRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < recoveryTriggerRequestPro.osdlist_size(); ++i) {
			_osdList.push_back(recoveryTriggerRequestPro.osdlist(i));
	}
	for (int i = 0; i < recoveryTriggerRequestPro.dstosdlist_size(); ++i) {
			_dstSpecOsdList.push_back(recoveryTriggerRequestPro.dstosdlist(i));
	}
	_dstSpecified = recoveryTriggerRequestPro.dstspecified();
	return;

}

void RecoveryTriggerRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->recoveryTriggerProcessor (_msgHeader.requestId, _sockfd, _osdList, _dstSpecified, _dstSpecOsdList);
#endif
}

void RecoveryTriggerRequestMsg::setSegmentLocations(
	vector<struct SegmentLocation> objLocs) {
	_segmentLocations = objLocs;
}

vector<struct SegmentLocation> RecoveryTriggerRequestMsg::getSegmentLocations() {
	return _segmentLocations;
}

void RecoveryTriggerRequestMsg::printProtocol() {
	debug("%s\n", "GOT MESSAGE [RECOVERY_TRIGGER_REQUEST]");

}

