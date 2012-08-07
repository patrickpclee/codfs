#include "putobjectinitrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutObjectInitRequestMsg::PutObjectInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectInitRequestMsg::PutObjectInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t objectSize, uint32_t chunkCount) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_objectSize = objectSize;
	_chunkCount = chunkCount;
}

void PutObjectInitRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectInitRequestPro putObjectInitRequestPro;
	putObjectInitRequestPro.set_objectid(_objectId);
	putObjectInitRequestPro.set_objectsize(_objectSize);
	putObjectInitRequestPro.set_chunkcount(_chunkCount);

	if (!putObjectInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_OBJECT_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutObjectInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectInitRequestPro putObjectInitRequestPro;
	putObjectInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putObjectInitRequestPro.objectid();
	_objectSize = putObjectInitRequestPro.objectsize();
	_chunkCount = putObjectInitRequestPro.chunkcount();

}

void PutObjectInitRequestMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectInitProcessor (_msgHeader.requestId, _sockfd, _objectId, _objectSize, _chunkCount);
#endif

	MemoryPool::getInstance().poolFree(_recvBuf);
}

void PutObjectInitRequestMsg::printProtocol() {
	debug("[PUT_OBJECT_INIT_REQUEST] Object ID = %llu, Length = %llu, Count = %d\n",
			_objectId, _objectSize, _chunkCount);
}
