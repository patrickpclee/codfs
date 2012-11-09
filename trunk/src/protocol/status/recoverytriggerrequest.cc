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
		uint32_t mdsSockfd, const vector<uint32_t> &osdList) :
		Message(communicator) {

	_sockfd = mdsSockfd;
	_osdList = osdList;
}

void RecoveryTriggerRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryTriggerRequestPro recoveryTriggerRequestPro;
	for (uint32_t osd : _osdList) {
		recoveryTriggerRequestPro.add_osdlist(osd);
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
	return;

}

void RecoveryTriggerRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	//mds->recoverTriggerProcessor (_msgHeader.requestId, _sockfd, _osdList);
#endif
}

void RecoveryTriggerRequestMsg::printProtocol() {

}

