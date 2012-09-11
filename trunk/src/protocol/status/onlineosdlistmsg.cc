#include <iostream>
using namespace std;
#include "onlineosdlistmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

OnlineOsdListMsg::OnlineOsdListMsg(Communicator* communicator) :
		Message(communicator), _onlineOsdListRef(_onlineOsdList) {

}

OnlineOsdListMsg::OnlineOsdListMsg(Communicator* communicator, uint32_t sockfd,
		vector<struct OnlineOsd>& onlineOsdListRef) :
		Message(communicator), _onlineOsdListRef(onlineOsdListRef) {

	_sockfd = sockfd;
}

void OnlineOsdListMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OnlineOsdListPro onlineOsdListPro;

	vector<struct OnlineOsd>::iterator it;

	for (it = _onlineOsdListRef.begin(); it < _onlineOsdListRef.end(); ++it) {
		ncvfs::OnlineOsdPro* onlineOsdPro =
				onlineOsdListPro.add_onlineosdlist();
		onlineOsdPro->set_osdid((*it).osdId);
		onlineOsdPro->set_osdip((*it).osdIp);
		onlineOsdPro->set_osdport((*it).osdPort);
	}

	if (!onlineOsdListPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(ONLINE_OSD_LIST);
	setProtocolMsg(serializedString);

}

void OnlineOsdListMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OnlineOsdListPro onlineOsdListPro;
	onlineOsdListPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_onlineOsdList.clear();
	for (int i = 0; i < onlineOsdListPro.onlineosdlist_size(); ++i) {
		struct OnlineOsd tmpOnlineOsd;
		tmpOnlineOsd.osdId = onlineOsdListPro.onlineosdlist(i).osdid();
		tmpOnlineOsd.osdIp = onlineOsdListPro.onlineosdlist(i).osdip();
		tmpOnlineOsd.osdPort = onlineOsdListPro.onlineosdlist(i).osdport();
		_onlineOsdList.push_back(tmpOnlineOsd);
	}

}

void OnlineOsdListMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->OnlineOsdListProcessor(_msgHeader.requestId, _sockfd, _onlineOsdList);
#endif
}

void OnlineOsdListMsg::printProtocol() {
	debug("%s\n", "[ONLINE_OSD_LIST] built");
	for (uint32_t i = 0; i < _onlineOsdList.size(); i++) {
		debug(
				"[ONLINE_OSD_LIST] contains %" PRIu32 "  = (%" PRIu32 " %" PRIu32 " %" PRIu32 ")\n",
				i, _onlineOsdList[i].osdId, _onlineOsdList[i].osdIp, _onlineOsdList[i].osdPort);
	}
}
