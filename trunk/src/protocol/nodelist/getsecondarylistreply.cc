#include "getsecondarylistreply.hh"
#include "getsecondarylistrequest.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"
#include "../monitor/monitor.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MONITOR
extern Monitor* monitor;
#endif

GetSecondaryListReplyMsg::GetSecondaryListReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSecondaryListReplyMsg::GetSecondaryListReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, vector<struct SegmentLocation> secondaryList) :
		Message(communicator) {

	_sockfd = sockfd;
	_secondaryList = secondaryList;
}

void GetSecondaryListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSecondaryListReplyPro getSecondaryListReplyPro;

	for (struct SegmentLocation secondary : _secondaryList) {
		getSecondaryListReplyPro.add_secondarylist(secondary);
	}

	if (!getSecondaryListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SECONDARY_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetSecondaryListReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSecondaryListReplyPro getSecondaryListReplyPro;
	getSecondaryListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getSecondaryListReplyPro.secondarylist_size(); ++i) {
		_secondaryList.push_back(getSecondaryListReplyPro.secondarylist(i));
	}
	return;
}

void GetSecondaryListReplyMsg::doHandle() {
	GetSecondaryListRequestMsg* getSecondaryListRequestMsg =
			(GetSecondaryListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getSecondaryListRequestMsg->setSecondaryList(_secondaryList);
	getSecondaryListRequestMsg->setStatus(READY);
}

void GetSecondaryListReplyMsg::printProtocol() {
	debug("%s\n", "[GET_SECONDARY_LIST_REPLY] GOT.");
}
