#include <iostream>
#include "getsegmentidlistrequest.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

GetSegmentIdListRequestMsg::GetSegmentIdListRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
GetSegmentIdListRequestMsg::GetSegmentIdListRequestMsg(Communicator* communicator, uint32_t mdsSockfd, uint32_t clientId, uint32_t numOfObjs) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_clientId = clientId;
	_numOfObjs = numOfObjs;
}

void GetSegmentIdListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentIdListRequestPro getSegmentIdListRequestPro;

	getSegmentIdListRequestPro.set_clientid(_clientId);
	getSegmentIdListRequestPro.set_numofobjs(_numOfObjs);

	if (!getSegmentIdListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_ID_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void GetSegmentIdListRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentIdListRequestPro getSegmentIdListRequestPro;
	getSegmentIdListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = getSegmentIdListRequestPro.clientid();
	_numOfObjs = getSegmentIdListRequestPro.numofobjs();
}

void GetSegmentIdListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->getSegmentIdListProcessor(_msgHeader.requestId, _sockfd, _clientId, _numOfObjs);
#endif
}

void GetSegmentIdListRequestMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_ID_LIST_REQUEST] Number of Segments = %" PRIu32 "\n", _numOfObjs);
}

void GetSegmentIdListRequestMsg::setNumOfObjs (uint32_t numOfObjs)
{
	_numOfObjs = numOfObjs;
}

uint32_t GetSegmentIdListRequestMsg::getNumOfObjs ()
{
	return _numOfObjs;
}

void GetSegmentIdListRequestMsg::setSegmentIdList (vector<uint64_t> segmentIdList)
{
	_segmentIdList = segmentIdList;
}

vector<uint64_t> GetSegmentIdListRequestMsg::getSegmentIdList ()
{
	return _segmentIdList;
}

void GetSegmentIdListRequestMsg::setPrimaryList (vector<uint32_t> primaryList)
{
	_primaryList = primaryList;
}

vector<uint32_t> GetSegmentIdListRequestMsg::getPrimaryList ()
{
	return _primaryList;
}
