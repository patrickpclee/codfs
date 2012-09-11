#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/segmentlocation.hh"
#include "getsecondarylistrequest.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetSecondaryListRequestMsg::GetSecondaryListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSecondaryListRequestMsg::GetSecondaryListRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t numOfSegs) :
		Message(communicator) {

	_sockfd = mdsSockfd;
	_numOfSegs = numOfSegs;
	
}

void GetSecondaryListRequestMsg::prepareProtocolMsg() {
	string serializedString;


	ncvfs::GetSecondaryListRequestPro getSecondaryListRequestPro;
	getSecondaryListRequestPro.set_numofsegs(_numOfSegs);

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


}

void GetSecondaryListRequestMsg::doHandle() {

#ifdef COMPILE_FOR_MONITOR
	monitor->getSecondaryListProcessor (_msgHeader.requestId, _sockfd, _numOfSegs);
#endif

}

void GetSecondaryListRequestMsg::printProtocol() {
	debug("[GET_SECONDARY_LIST_REQUEST] NUMBER OF SEGS = %" PRIu32 "\n",_numOfSegs);
}

void GetSecondaryListRequestMsg::setSecondaryList(vector<struct SegmentLocation> secondaryList) {

	_secondaryList = secondaryList;
	return;

}
vector<struct SegmentLocation> GetSecondaryListRequestMsg::getSecondaryList() {


	return _secondaryList;

}

