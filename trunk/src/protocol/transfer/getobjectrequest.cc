#include "getobjectrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

GetObjectRequestMsg::GetObjectRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetObjectRequestMsg::GetObjectRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_objectId = objectId;
}

void GetObjectRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectRequestPro getObjectRequestPro;
	getObjectRequestPro.set_objectid(_objectId);

	if (!getObjectRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OBJECT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetObjectRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectRequestPro getObjectRequestPro;
	getObjectRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getObjectRequestPro.objectid();
	_objectSize = getObjectRequestPro.objectsize();
	_chunkCount = getObjectRequestPro.chunkcount();

}

void GetObjectRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getObjectRequestProcessor (_msgHeader.requestId, _sockfd, _objectId);
#endif
}

void GetObjectRequestMsg::setObjectSize(uint32_t objectSize){
	_objectSize = objectSize;
}

void GetObjectRequestMsg::setChunkCount(uint32_t chunkCount){
	_chunkCount = chunkCount;
}

uint32_t GetObjectRequestMsg::getObjectSize(){
	return _objectSize;
}

uint32_t GetObjectRequestMsg::getChunkCount(){
	return _chunkCount;
}

uint32_t GetObjectRequestMsg::getRequestId(){
	return _msgHeader.requestId;
}

void GetObjectRequestMsg::printProtocol() {
	debug("[GET_OBJECT_REQUEST] Object ID = %" PRIu64 "\n", _objectId);
}
