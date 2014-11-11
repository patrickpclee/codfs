#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "getprimarylistrequest.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetPrimaryListRequestMsg::GetPrimaryListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetPrimaryListRequestMsg::GetPrimaryListRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t numOfObjs) :
		Message(communicator) {

	_sockfd = mdsSockfd;
	_numOfObjs = numOfObjs;
}

void GetPrimaryListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetPrimaryListRequestPro getPrimaryListRequestPro;
	getPrimaryListRequestPro.set_numofobjs(_numOfObjs);

	if (!getPrimaryListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_PRIMARY_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void GetPrimaryListRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetPrimaryListRequestPro getPrimaryListRequestPro;
	getPrimaryListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_numOfObjs = getPrimaryListRequestPro.numofobjs();

}

void GetPrimaryListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getPrimaryListProcessor (_msgHeader.requestId, _sockfd, _numOfObjs);
#endif
}

void GetPrimaryListRequestMsg::printProtocol() {
	debug("[GET_PRIMARY_LIST_REQUEST] NUMBER OF OBJS = %" PRIu32 "\n",_numOfObjs);
}

void GetPrimaryListRequestMsg::setPrimaryList(vector<uint32_t> primaryList) {
	_primaryList = primaryList;
	return;
}

vector<uint32_t> GetPrimaryListRequestMsg::getPrimaryList() {
	return _primaryList;
}
