#include <iostream>
#include "getobjectidlistrequest.hh"
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

GetObjectIdListRequestMsg::GetObjectIdListRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
GetObjectIdListRequestMsg::GetObjectIdListRequestMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t numOfObjs) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_numOfObjs = numOfObjs;
}

void GetObjectIdListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectIdListRequestPro getObjectIdListRequestPro;

	getObjectIdListRequestPro.set_numofobjs(_numOfObjs);

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OBJECT_ID_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void GetObjectIdListRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectIdListRequestPro getObjectIdListRequestPro;
	getObjectIdListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_numOfObjs = getObjectIdListRequestPro.numofobjs();
}

void GetObjectIdListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->getObjectIdListProcessor(_msgHeader.requestId, _sockfd, _numOfObjs);
#endif
}

void GetObjectIdListRequestMsg::printProtocol() {
	debug(
			"[GET_OBJECT_ID_LIST_REQUEST] Number of Objects= %" PRIu32 "\n",
			_numOfObjs);
}

void GetObjectIdListRequestMsg::setNumOfObjs (uint32_t numOfObjs)
{
	_numOfObjs = numOfObjs;
}

uint32_t GetObjectIdListRequestMsg::getNumOfObjs ()
{
	return _numOfObjs;
}

void GetObjectIdListRequestMsg::setObjectIdList (vector<uint64_t> objectIdList)
{
	_objectIdList = objectIdList;
}

vector<uint64_t> GetObjectIdListRequestMsg::getObjectIdList ()
{
	return _objectIdList;
}

void GetObjectIdListRequestMsg::setPrimaryList (vector<uint32_t> primaryList)
{
	_primaryList = primaryList;
}

vector<uint32_t> GetObjectIdListRequestMsg::getPrimaryList ()
{
	return _primaryList;
}
