#include <iostream>
#include "getsegmentidlistrequest.hh"
#include "getsegmentidlistreply.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

/**
 * Default Constructor
 */

GetSegmentIdListReplyMsg::GetSegmentIdListReplyMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
GetSegmentIdListReplyMsg::GetSegmentIdListReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t mdsSockfd, const vector<uint64_t> &segmentIdList, const vector<uint32_t> &primaryList) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = mdsSockfd;
	_segmentIdList = segmentIdList;
	_primaryList = primaryList;
	
}

void GetSegmentIdListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentIdListReplyPro getSegmentIdListReplyPro;

	//getSegmentIdListReplyPro.set_numofobjs(_numOfObjs);
	for (uint32_t i = 0; i < _segmentIdList.size(); ++i) {
		getSegmentIdListReplyPro.add_segmentidlist(_segmentIdList[i]);
		getSegmentIdListReplyPro.add_primarylist(_primaryList[i]);
	}

	if (!getSegmentIdListReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_ID_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetSegmentIdListReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentIdListReplyPro getSegmentIdListReplyPro;
	getSegmentIdListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getSegmentIdListReplyPro.segmentidlist_size(); i++) {
		_segmentIdList.push_back(getSegmentIdListReplyPro.segmentidlist(i));
		_primaryList.push_back(getSegmentIdListReplyPro.primarylist(i));
	}
}

void GetSegmentIdListReplyMsg::doHandle() {
	GetSegmentIdListRequestMsg* getSegmentIdListRequestMsg =
			(GetSegmentIdListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getSegmentIdListRequestMsg->setSegmentIdList(_segmentIdList);
	getSegmentIdListRequestMsg->setPrimaryList(_primaryList);
	getSegmentIdListRequestMsg->setStatus(READY);
}

void GetSegmentIdListReplyMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_ID_LIST_REPLY] Number of Segments = %zu\n",
			_segmentIdList.size());
}

/*
void GetSegmentIdListReplyMsg::setSegmentIdList (vector<uint64_t> segmentIdList)
{
	_segmentIdList = segmentIdList;
}

vector<uint64_t> GetSegmentIdListReplyMsg::getSegmentIdList ()
{
	return _segmentIdList;
}
*/
