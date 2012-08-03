#include "putsegmentinit.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutSegmentInitMsg::PutSegmentInitMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitMsg::PutSegmentInitMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId, uint32_t segmentSize, uint32_t chunkCount) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_chunkCount = chunkCount;
}

void PutSegmentInitMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitPro putSegmentInitPro;
	putSegmentInitPro.set_objectid(_objectId);
	putSegmentInitPro.set_segmentid(_segmentId);
	putSegmentInitPro.set_segmentsize(_segmentSize);
	putSegmentInitPro.set_chunkcount(_chunkCount);

	if (!putSegmentInitPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_SEGMENT_INIT);
	setProtocolMsg(serializedString);

}

void PutSegmentInitMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentInitPro putSegmentInitPro;
	putSegmentInitPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putSegmentInitPro.objectid();
	_segmentId = putSegmentInitPro.segmentid();
	_segmentSize = putSegmentInitPro.segmentsize();
	_chunkCount = putSegmentInitPro.chunkcount();

}

void PutSegmentInitMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentInitProcessor (_msgHeader.requestId, _sockfd, _objectId, _segmentId, _segmentSize, _chunkCount);
#endif
}

void PutSegmentInitMsg::printProtocol() {
	debug("[PUT_SEGMENT_INIT] Object ID = %lu, Segment ID = %u, Length = %u, Count = %d\n",
			_objectId, _segmentId, _segmentSize, _chunkCount);
}
