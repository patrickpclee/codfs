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
GetObjectIdListReplyMsg::GetObjectIdListReplyMsg(Communicator* communicator,
		uint32_t mdsSockfd, vector<uint64_t> objectIdList) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_objectIdList = objectIdList;
	_threadPoolLevel = 1;
}

void GetObjectIdListReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectIdListReplyPro getObjectIdListReplyPro;

	//getObjectIdListReplyPro.set_numofobjs(_numOfObjs);
	for (auto objectId : _objectIdList) {
		getObjectIdListReplyPro.add_objectidlist(objectId);
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
	}
}

void GetObjectIdListReplyMsg::doHandle() {
	GetObjectIdListRequestMsg* getObjectIdListRequestMsg =
			(GetObjectIdListRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getObjectIdListRequestMsg->setObjectIdList(_objectIdList);
	getObjectIdListRequestMsg->setStatus(READY);
}

void GetObjectIdListReplyMsg::printProtocol() {
	debug(
			"[GET_OBJECT_ID_LIST_REPLY] Number of Objects= %zu\n",
			_objectIdList.size());
}

void GetObjectIdListReplyMsg::setObjectIdList (vector<uint64_t> objectIdList)
{
	_objectIdList = objectIdList;
}

vector<uint64_t> GetObjectIdListReplyMsg::getObjectIdList ()
{
	return _objectIdList;
}
