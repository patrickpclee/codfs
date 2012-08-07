#include "putsegmentinitrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId, uint32_t segmentSize, uint32_t chunkCount) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_chunkCount = chunkCount;
}

void PutSegmentInitRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitRequestPro putSegmentInitRquestPro;
	putSegmentInitRquestPro.set_objectid(_objectId);
	putSegmentInitRquestPro.set_segmentid(_segmentId);
	putSegmentInitRquestPro.set_segmentsize(_segmentSize);
	putSegmentInitRquestPro.set_chunkcount(_chunkCount);

	if (!putSegmentInitRquestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_SEGMENT_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutSegmentInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentInitRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putSegmentInitRequestPro.objectid();
	_segmentId = putSegmentInitRequestPro.segmentid();
	_segmentSize = putSegmentInitRequestPro.segmentsize();
	_chunkCount = putSegmentInitRequestPro.chunkcount();

}

void PutSegmentInitRequestMsg::handle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentInitProcessor (_msgHeader.requestId, _sockfd, _objectId, _segmentId, _segmentSize, _chunkCount);
#endif
}

void PutSegmentInitRequestMsg::printProtocol() {
	debug("[PUT_SEGMENT_INIT_REQUEST] Object ID = %llu, Segment ID = %u, Length = %u, Count = %d\n",
			_objectId, _segmentId, _segmentSize, _chunkCount);
}
