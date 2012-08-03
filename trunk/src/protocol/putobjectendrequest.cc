#include "putobjectendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutObjectEndRequestMsg::PutObjectEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectEndRequestMsg::PutObjectEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
}

void PutObjectEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectEndRequestPro putObjectEndRequestPro;
	putObjectEndRequestPro.set_objectid(_objectId);

	if (!putObjectEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_OBJECT_END_REQUEST);
	setProtocolMsg(serializedString);

}

void PutObjectEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectEndRequestPro putObjectEndRequestPro;
	putObjectEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putObjectEndRequestPro.objectid();

}

void PutObjectEndRequestMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectEndProcessor (_msgHeader.requestId, _sockfd, _objectId);
#endif

	MemoryPool::getInstance().poolFree(_recvBuf);
}

void PutObjectEndRequestMsg::printProtocol() {
	debug("[PUT_OBJECT_END_REQUEST] Object ID = %lu\n", _objectId);
}
