#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/blocklocation.hh"
#include "getsecondarylistrequest.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetSecondaryListRequestMsg::GetSecondaryListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSecondaryListRequestMsg::GetSecondaryListRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t numOfSegs, uint32_t primaryId, uint64_t
		blockSize) :
		Message(communicator) {

	_sockfd = mdsSockfd;
	_numOfSegs = numOfSegs;
	_primaryId = primaryId;
	_blockSize = blockSize;
	
}

void GetSecondaryListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSecondaryListRequestPro getSecondaryListRequestPro;
	getSecondaryListRequestPro.set_numofsegs(_numOfSegs);
	getSecondaryListRequestPro.set_primaryid(_primaryId);
	getSecondaryListRequestPro.set_blocksize(_blockSize);

	if (!getSecondaryListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_SECONDARY_LIST_REQUEST);
	setProtocolMsg(serializedString);


}

void GetSecondaryListRequestMsg::parse(char* buf) {


	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSecondaryListRequestPro getSecondaryListRequestPro;
	getSecondaryListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_numOfSegs = getSecondaryListRequestPro.numofsegs();
	_primaryId = getSecondaryListRequestPro.primaryid();
	_blockSize = getSecondaryListRequestPro.blocksize();

}

void GetSecondaryListRequestMsg::doHandle() {

#ifdef COMPILE_FOR_MONITOR
	monitor->getSecondaryListProcessor (_msgHeader.requestId, _sockfd, _numOfSegs, _primaryId, _blockSize);
#endif

}

void GetSecondaryListRequestMsg::printProtocol() {
	debug("[GET_SECONDARY_LIST_REQUEST] NUMBER OF SEGS = %" PRIu32 " PRIMARY ID = %" PRIu32 " BLOCK SIZE = %"
	 PRIu64 "\n",
	_numOfSegs, _primaryId, _blockSize);
}

void GetSecondaryListRequestMsg::setSecondaryList(vector<struct BlockLocation> secondaryList) {
	_secondaryList = secondaryList;
	return;
}

vector<struct BlockLocation> GetSecondaryListRequestMsg::getSecondaryList() {
	return _secondaryList;
}

