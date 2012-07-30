#include "putobjectinit.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutObjectInitMsg::PutObjectInitMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectInitMsg::PutObjectInitMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t objectSize) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_objectSize = objectSize;
}

void PutObjectInitMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectInitPro putObjectInitPro;
	putObjectInitPro.set_objectid(_objectId);
	putObjectInitPro.set_objectsize(_objectSize);

	if (!putObjectInitPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_OBJECT_INIT);
	setProtocolMsg(serializedString);
}

void PutObjectInitMsg::parse(char* buf) {

	// DEBUG
	printhex (buf + sizeof (struct MsgHeader), _msgHeader.protocolMsgSize);

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutObjectInitPro putObjectInitPro;
	putObjectInitPro.ParseFromString(buf + sizeof(struct MsgHeader));

	_objectId = putObjectInitPro.objectid();
	_objectSize = putObjectInitPro.objectsize();

}

void PutObjectInitMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectInitProcessor (_sockfd, _objectId, _objectSize);
#endif
}

void PutObjectInitMsg::printProtocol() {
	cout << "[PUT_OBJECT_INIT] objectID = " << _objectId << " Length = "
			<< _objectSize << endl;
}
