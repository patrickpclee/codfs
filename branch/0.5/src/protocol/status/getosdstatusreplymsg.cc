#include <iostream>
using namespace std;
#include "getosdstatusreplymsg.hh"
#include "getosdstatusrequestmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetOsdStatusReplyMsg::GetOsdStatusReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetOsdStatusReplyMsg::GetOsdStatusReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, const vector<bool>
		&osdStatusRef) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_osdStatus = osdStatusRef;
	
}

void GetOsdStatusReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetOsdStatusReplyPro getOsdStatusReplyPro;

	for (bool stat : _osdStatus) {
		getOsdStatusReplyPro.add_osdstatus(stat);
	}

	if (!getOsdStatusReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OSD_STATUS_REPLY);
	setProtocolMsg(serializedString);

}

void GetOsdStatusReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdStatusReplyPro getOsdStatusReplyPro;
	getOsdStatusReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getOsdStatusReplyPro.osdstatus_size(); ++i) {
		_osdStatus.push_back(getOsdStatusReplyPro.osdstatus(i));
	}
	return;
}

void GetOsdStatusReplyMsg::doHandle() {
	GetOsdStatusRequestMsg* getOsdStatusRequestMsg =
			(GetOsdStatusRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getOsdStatusRequestMsg->setOsdStatus(_osdStatus);
	getOsdStatusRequestMsg->setStatus(READY);
}

void GetOsdStatusReplyMsg::printProtocol() {
	debug("%s\n", "[GET_OSD_STATUS_REPLY] GOT.");
}
