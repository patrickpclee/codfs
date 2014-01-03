#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "getosdlistrequest.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetOsdListRequestMsg::GetOsdListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetOsdListRequestMsg::GetOsdListRequestMsg(Communicator* communicator,
		uint32_t sockfd) :
		Message(communicator) {

	_sockfd = sockfd;
}

void GetOsdListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetOsdListRequestPro getOsdListRequestPro;

	if (!getOsdListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OSD_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void GetOsdListRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdListRequestPro getOsdListRequestPro;
	getOsdListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

}

void GetOsdListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getOsdListProcessor (_msgHeader.requestId, _sockfd);
#endif
}

void GetOsdListRequestMsg::printProtocol() {
	debug("%s\n", "[GET_OSD_LIST_REQUEST]");
}

void GetOsdListRequestMsg::setOsdList(vector<struct OnlineOsd>& _osdList,
	vector<struct OnlineOsd>& osdList) {
	_osdList = osdList;
	return;
}

vector<struct OnlineOsd>& GetOsdListRequestMsg::getOsdList() {
	return _osdList;
}
