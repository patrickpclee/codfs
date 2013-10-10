#include <iostream>
using namespace std;
#include "getsegmentcodinginforequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

GetSegmentCodingInfoRequestMsg::GetSegmentCodingInfoRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentCodingInfoRequestMsg::GetSegmentCodingInfoRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, list<uint64_t> segmentIdList) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_segmentIdList = segmentIdList;
}

void GetSegmentCodingInfoRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentCodingInfoRequestPro getSegmentCodingInfoRequestPro;

	for (uint64_t segmentId : _segmentIdList) {
		getSegmentCodingInfoRequestPro.add_segmentidlist(segmentId);
	}

	if (!getSegmentCodingInfoRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_CODING_INFO_REQUEST);
	setProtocolMsg(serializedString);

}

void GetSegmentCodingInfoRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentCodingInfoRequestPro getSegmentCodingInfoRequestPro;
	getSegmentCodingInfoRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getSegmentCodingInfoRequestPro.segmentidlist().size(); ++i) {
		_segmentIdList.push_back(getSegmentCodingInfoRequestPro.segmentidlist(i));
	}

}

void GetSegmentCodingInfoRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->getSegmentCodingInfoProcessor(_msgHeader.requestId, _sockfd, _segmentIdList);
#endif
}

void GetSegmentCodingInfoRequestMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_CODING_INFO_REQUEST] Segment ID List Size = %zu\n",
			_segmentIdList.size());
}
