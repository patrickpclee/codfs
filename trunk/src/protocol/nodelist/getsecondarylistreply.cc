#include <iostream>
using namespace std;
#include "getsecondarylistreply.hh"
#include "getsecondarylistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetSecondaryListReplyMsg::GetSecondaryListReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSecondaryListReplyMsg::GetSecondaryListReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd,
		const vector<struct BlockLocation> &secondaryList) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_secondaryList = secondaryList;
}

void GetSecondaryListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSecondaryListReplyPro getSecondaryListReplyPro;

	vector<BlockLocation>::iterator it;

	for (it = _secondaryList.begin(); it < _secondaryList.end(); ++it) {
		ncvfs::BlockLocationPro* blockLocationPro =
				getSecondaryListReplyPro.add_secondarylist();
		blockLocationPro->set_osdid((*it).osdId);
		blockLocationPro->set_blockid((*it).blockId);
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
		struct BlockLocation tempBlockLocation;

		tempBlockLocation.osdId =
				getSecondaryListReplyPro.secondarylist(i).osdid();
		tempBlockLocation.blockId =
				getSecondaryListReplyPro.secondarylist(i).blockid();

		_secondaryList.push_back(tempBlockLocation);
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
