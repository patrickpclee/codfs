#include "putobjectend.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutObjectEndMsg::PutObjectEndMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectEndMsg::PutObjectEndMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
}

void PutObjectEndMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectEndPro putObjectEndPro;
	putObjectEndPro.set_objectid(_objectId);

	if (!putObjectEndPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_OBJECT_END);
	setProtocolMsg(serializedString);

}

void PutObjectEndMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectEndPro putObjectEndPro;
	putObjectEndPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putObjectEndPro.objectid();

}

void PutObjectEndMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectEndProcessor (_msgHeader.requestId, _sockfd, _objectId);
#endif
}

void PutObjectEndMsg::printProtocol() {
	debug ("[PUT_OBJECT_END] Object ID = %lu\n", _objectId);
}
