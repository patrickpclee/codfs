#include <iostream>
#include "switchprimaryosdrequestmsg.hh"
#include "switchprimaryosdreplymsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

using namespace std;
SwitchPrimaryOsdReplyMsg::SwitchPrimaryOsdReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

SwitchPrimaryOsdReplyMsg::SwitchPrimaryOsdReplyMsg(Communicator* communicator,
	uint32_t requestId,  uint32_t sockfd, uint32_t osdId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_newPrimaryOsdId = osdId;
}

void SwitchPrimaryOsdReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SwitchPrimaryOsdReplyPro switchPrimaryOsdReplyPro;
	switchPrimaryOsdReplyPro.set_newprimaryosdid(_newPrimaryOsdId);

	if (!switchPrimaryOsdReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (SWITCH_PRIMARY_OSD_REPLY);
	setProtocolMsg(serializedString);

}

void SwitchPrimaryOsdReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SwitchPrimaryOsdReplyPro switchPrimaryOsdReplyPro;
	switchPrimaryOsdReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_newPrimaryOsdId = switchPrimaryOsdReplyPro.newprimaryosdid();

}

void SwitchPrimaryOsdReplyMsg::doHandle() {
	SwitchPrimaryOsdRequestMsg* req = (SwitchPrimaryOsdRequestMsg*)
	_communicator->popWaitReplyMessage(_msgHeader.requestId);
	req->setNewPrimaryOsdId(_newPrimaryOsdId);
	req->setStatus(READY);
}

void SwitchPrimaryOsdReplyMsg::printProtocol() {
	debug("[SWITCH PRIMARY OSD REPLY] New Primary Id = %" PRIu32 "\n",
			_newPrimaryOsdId);
}
