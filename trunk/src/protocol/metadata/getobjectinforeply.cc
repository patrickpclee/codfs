#include <iostream>
#include "getobjectinforequest.hh"
#include "getobjectinforeply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetObjectInfoReplyMsg::GetObjectInfoReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetObjectInfoReplyMsg::GetObjectInfoReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t objectId,
		uint32_t objectSize, vector<uint32_t> nodeList,
		CodingScheme codingScheme, string codingSetting) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;

	_objectId = objectId;
	_objectSize = objectSize;
	_nodeList = nodeList;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;

}

void GetObjectInfoReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectInfoReplyPro getObjectInfoReplyPro;
	getObjectInfoReplyPro.set_objectid(_objectId);
	getObjectInfoReplyPro.set_codingscheme(
			(ncvfs::PutObjectInitRequestPro_CodingScheme) _codingScheme);
	getObjectInfoReplyPro.set_codingsetting(_codingSetting);

	for (auto nodeID : _nodeList) {
		getObjectInfoReplyPro.add_nodelist(nodeID);
	}

	if (!getObjectInfoReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OBJECT_INFO_REPLY);
	setProtocolMsg(serializedString);

}

void GetObjectInfoReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectInfoReplyPro getObjectInfoReplyPro;
	getObjectInfoReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getObjectInfoReplyPro.objectid();
	_objectSize = getObjectInfoReplyPro.objectsize();
	_codingScheme = (CodingScheme) getObjectInfoReplyPro.codingscheme();
	_codingSetting = getObjectInfoReplyPro.codingsetting();

	for (int i = 0; i < getObjectInfoReplyPro.nodelist_size(); i++) {
		_nodeList.push_back(getObjectInfoReplyPro.nodelist(i));
	}

}

void GetObjectInfoReplyMsg::doHandle() {
	GetObjectInfoRequestMsg* getObjectInfoRequestMsg =
			(GetObjectInfoRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getObjectInfoRequestMsg->setCodingScheme(_codingScheme);
	getObjectInfoRequestMsg->setCodingSetting(_codingSetting);
	getObjectInfoRequestMsg->setNodeList(_nodeList);
	getObjectInfoRequestMsg->setObjectSize(_objectSize);
	getObjectInfoRequestMsg->setStatus(READY);
}

void GetObjectInfoReplyMsg::printProtocol() {
	debug(
			"[GET_OBJECT_INFO_REPLY] Object ID = %" PRIu64 " Size = %" PRIu32 " CodingScheme = %d Setting = %s\n",
			_objectId, _objectSize, (int)_codingScheme, _codingSetting.c_str());
}
