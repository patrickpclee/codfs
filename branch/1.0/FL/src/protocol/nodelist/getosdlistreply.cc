#include <iostream>
using namespace std;
#include "getosdlistreply.hh"
#include "getosdlistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetOsdListReplyMsg::GetOsdListReplyMsg(Communicator* communicator) :
		Message(communicator), _osdListRef(_osdList) {

}

GetOsdListReplyMsg::GetOsdListReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, vector<struct OnlineOsd>& osdListRef) :
		Message(communicator), _osdListRef(osdListRef) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
}

void GetOsdListReplyMsg::prepareProtocolMsg() {
	string serializedString;


	ncvfs::GetOsdListReplyPro getOsdListReplyPro;

	vector<struct OnlineOsd>::iterator it;

	for (it = _osdListRef.begin(); it < _osdListRef.end(); ++it) {
		ncvfs::OnlineOsdPro* onlineOsdPro = 
			getOsdListReplyPro.add_onlineosdlist();
		onlineOsdPro->set_osdid((*it).osdId);
		onlineOsdPro->set_osdip((*it).osdIp);
		onlineOsdPro->set_osdport((*it).osdPort);
	}

	if (!getOsdListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OSD_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetOsdListReplyMsg::parse(char* buf) {


	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdListReplyPro getOsdListReplyPro;
	getOsdListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdList.clear();
	for (int i = 0; i < getOsdListReplyPro.onlineosdlist_size(); ++i) {
		struct OnlineOsd tmpOnlineOsd;
		tmpOnlineOsd.osdId = getOsdListReplyPro.onlineosdlist(i).osdid();
		tmpOnlineOsd.osdIp = getOsdListReplyPro.onlineosdlist(i).osdip();
		tmpOnlineOsd.osdPort = getOsdListReplyPro.onlineosdlist(i).osdport();
		_osdList.push_back(tmpOnlineOsd);
	}
	return;

}

void GetOsdListReplyMsg::doHandle() {

	GetOsdListRequestMsg* getOsdListRequestMsg =
			(GetOsdListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getOsdListRequestMsg->setOsdList(getOsdListRequestMsg->getOsdList(), _osdList);
	getOsdListRequestMsg->setStatus(READY);

}

void GetOsdListReplyMsg::printProtocol() {
	debug("%s\n", "[GET_OSD_LIST_REPLY] GOT.");
	for (uint32_t i = 0; i < (uint32_t) _osdList.size(); i++) {
		debug("LIST i=%" PRIu32 " ip = %" PRIu32 " port = %" PRIu32 "\n",i, _osdList[i].osdIp, _osdList[i].osdPort);
	}
}
