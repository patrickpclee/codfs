#include <iostream>
#include "getobjectidlistrequest.hh"
#include "getobjectidlistreply.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

/**
 * Default Constructor
 */

GetObjectIdListReplyMsg::GetObjectIdListReplyMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
GetObjectIdListReplyMsg::GetObjectIdListReplyMsg(Communicator* communicator, uint32_t requestId, uint32_t mdsSockfd, vector<uint64_t> objectIdList, vector<uint32_t> primaryList) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = mdsSockfd;
	_objectIdList = objectIdList;
	_primaryList = primaryList;
	
}

void GetObjectIdListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectIdListReplyPro getObjectIdListReplyPro;

	//getObjectIdListReplyPro.set_numofobjs(_numOfObjs);
	for (uint32_t i = 0; i < _objectIdList.size(); ++i) {
		getObjectIdListReplyPro.add_objectidlist(_objectIdList[i]);
		getObjectIdListReplyPro.add_primarylist(_primaryList[i]);
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OBJECT_ID_LIST_REPLY);
	setProtocolMsg(serializedString);

}

void GetObjectIdListReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectIdListReplyPro getObjectIdListReplyPro;
	getObjectIdListReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	for (int i = 0; i < getObjectIdListReplyPro.objectidlist_size(); i++) {
		_objectIdList.push_back(getObjectIdListReplyPro.objectidlist(i));
		_primaryList.push_back(getObjectIdListReplyPro.primarylist(i));
	}
}

void GetObjectIdListReplyMsg::doHandle() {
	GetObjectIdListRequestMsg* getObjectIdListRequestMsg =
			(GetObjectIdListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getObjectIdListRequestMsg->setObjectIdList(_objectIdList);
	getObjectIdListRequestMsg->setPrimaryList(_primaryList);
	getObjectIdListRequestMsg->setStatus(READY);
}

void GetObjectIdListReplyMsg::printProtocol() {
	debug(
			"[GET_OBJECT_ID_LIST_REPLY] Number of Objects= %zu\n",
			_objectIdList.size());
}

/*
void GetObjectIdListReplyMsg::setObjectIdList (vector<uint64_t> objectIdList)
{
	_objectIdList = objectIdList;
}

vector<uint64_t> GetObjectIdListReplyMsg::getObjectIdList ()
{
	return _objectIdList;
}
*/
