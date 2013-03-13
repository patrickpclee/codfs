#include <iostream>
using namespace std;
#include "getprimarylistreply.hh"
#include "getprimarylistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetPrimaryListReplyMsg::GetPrimaryListReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetPrimaryListReplyMsg::GetPrimaryListReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t mdsSockfd, const vector<uint32_t> &primaryList) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = mdsSockfd;
	_primaryList = primaryList;
	
}

void GetPrimaryListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetPrimaryListReplyPro getPrimaryListReplyPro;

	for (uint32_t primary : _primaryList) {
		getPrimaryListReplyPro.add_primarylist(primary);
	}

	if (!getPrimaryListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_PRIMARY_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetPrimaryListReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetPrimaryListReplyPro getPrimaryListReplyPro;
	getPrimaryListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getPrimaryListReplyPro.primarylist_size(); ++i) {
		_primaryList.push_back(getPrimaryListReplyPro.primarylist(i));
	}
	return;
}

void GetPrimaryListReplyMsg::doHandle() {
	GetPrimaryListRequestMsg* getPrimaryListRequestMsg =
			(GetPrimaryListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getPrimaryListRequestMsg->setPrimaryList(_primaryList);
	getPrimaryListRequestMsg->setStatus(READY);
}

void GetPrimaryListReplyMsg::printProtocol() {
	debug("%s\n", "[GET_PRIMARY_LIST_REPLY] GOT.");
}
