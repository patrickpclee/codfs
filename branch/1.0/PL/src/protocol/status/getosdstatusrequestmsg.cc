#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "getosdstatusrequestmsg.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetOsdStatusRequestMsg::GetOsdStatusRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetOsdStatusRequestMsg::GetOsdStatusRequestMsg(Communicator* communicator,
		uint32_t sockfd, vector<uint32_t>& osdListRef) :
		Message(communicator) {

	_sockfd = sockfd;
	_osdList = osdListRef;
}

void GetOsdStatusRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetOsdStatusRequestPro getOsdStatusRequestPro;
	for (uint32_t osdid : _osdList) {
		getOsdStatusRequestPro.add_osdids(osdid);
	}

	if (!getOsdStatusRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OSD_STATUS_REQUEST);
	setProtocolMsg(serializedString);

}

void GetOsdStatusRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdStatusRequestPro getOsdStatusRequestPro;
	getOsdStatusRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getOsdStatusRequestPro.osdids_size(); ++i) {
		_osdList.push_back(getOsdStatusRequestPro.osdids(i));
	}

}

void GetOsdStatusRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getOsdStatusRequestProcessor (_msgHeader.requestId, _sockfd, _osdList);
#endif
}

void GetOsdStatusRequestMsg::printProtocol() {
	debug("%s\n", "[GET_OSD_STATUS_REQUEST]");
}

void GetOsdStatusRequestMsg::setOsdStatus(vector<bool>& osdStatusRef) {
	_osdStatus = osdStatusRef;
	return;
}

vector<bool>& GetOsdStatusRequestMsg::getOsdStatus() {
	return _osdStatus;
}
