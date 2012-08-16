#include "putobjectinitrequest.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutObjectInitRequestMsg::PutObjectInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutObjectInitRequestMsg::PutObjectInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t objectSize,
		uint32_t chunkCount, CodingScheme codingScheme, string codingSetting) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_objectSize = objectSize;
	_chunkCount = chunkCount;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
}

void PutObjectInitRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutObjectInitRequestPro putObjectInitRequestPro;
	putObjectInitRequestPro.set_objectid(_objectId);
	putObjectInitRequestPro.set_objectsize(_objectSize);
	putObjectInitRequestPro.set_chunkcount(_chunkCount);
	putObjectInitRequestPro.set_codingscheme(
			(ncvfs::PutObjectInitRequestPro_CodingScheme) _codingScheme);
	putObjectInitRequestPro.set_codingsetting(_codingSetting);

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
	_codingScheme = (CodingScheme) putObjectInitRequestPro.codingscheme();
	_codingSetting = putObjectInitRequestPro.codingsetting();

}

void PutObjectInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putObjectInitProcessor (_msgHeader.requestId, _sockfd, _objectId,
			_objectSize, _chunkCount, _codingScheme, _codingSetting);
#endif
}

void PutObjectInitRequestMsg::printProtocol() {
	debug(
			"[PUT_OBJECT_INIT_REQUEST] Object ID = %" PRIu64 ", Length = %" PRIu64 ", Count = %" PRIu32 " CodingScheme = %" PRIu32 " CodingSetting = %s\n",
			_objectId, _objectSize, _chunkCount, _codingScheme, _codingSetting.c_str());
}
