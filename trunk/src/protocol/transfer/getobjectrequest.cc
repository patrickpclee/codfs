#include "getobjectrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
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

}

void GetObjectRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getObjectRequestProcessor (_msgHeader.requestId, _sockfd, _objectId);
#endif
}

void GetObjectRequestMsg::printProtocol() {
	debug("[GET_OBJECT_REQUEST] Object ID = %" PRIu64 "\n", _objectId);
}
