#include "getobjectreplymsg.hh"
#include "getobjectrequest.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"

GetObjectReplyMsg::GetObjectReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetObjectReplyMsg::GetObjectReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t objectId, uint32_t
		objectSize, uint32_t chunkCount) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_objectId = objectId;
	_objectSize = objectSize;
	_chunkCount = chunkCount;
}

void GetObjectReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectReplyPro getObjectReplyPro;
	getObjectReplyPro.set_objectid(_objectId);
	getObjectReplyPro.set_objectsize(_objectSize);
	getObjectReplyPro.set_chunkcount(_chunkCount);

	if (!getObjectReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OBJECT_REPLY);
	setProtocolMsg(serializedString);

}

void GetObjectReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectReplyPro getObjectReplyPro;
	getObjectReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getObjectReplyPro.objectid();
	_objectSize = getObjectReplyPro.objectsize();
	_chunkCount = getObjectReplyPro.chunkcount();

}

void GetObjectReplyMsg::doHandle() {
	GetObjectRequestMsg* getObjectRequestMsg =
			(GetObjectRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getObjectRequestMsg->setObjectSize(_objectSize);
	getObjectRequestMsg->setChunkCount(_chunkCount);
	getObjectRequestMsg->setStatus(READY);
}

void GetObjectReplyMsg::printProtocol() {
	debug("[GET_OBJECT_REPLY] Object ID = %" PRIu64 " Object size = %" PRIu32 " Chunk count = %" PRIu32 "\n", _objectId, _objectSize, _chunkCount);
}
