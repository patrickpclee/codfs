#include "putsegmentendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

PutSegmentEndRequestMsg::PutSegmentEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentEndRequestMsg::PutSegmentEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void PutSegmentEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentEndRequestPro putSegmentEndRequestPro;
	putSegmentEndRequestPro.set_objectid(_objectId);
	putSegmentEndRequestPro.set_segmentid(_segmentId);

	if (!putSegmentEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_SEGMENT_END_REQUEST);
	setProtocolMsg(serializedString);

}

void PutSegmentEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentEndRequestPro putSegmentEndRequestPro;
	putSegmentEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = putSegmentEndRequestPro.objectid();
	_segmentId = putSegmentEndRequestPro.segmentid();

}

void PutSegmentEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentEndProcessor (_msgHeader.requestId, _sockfd, _objectId, _segmentId);
#endif
}

void PutSegmentEndRequestMsg::printProtocol() {
	debug("[PUT_SEGMENT_END_REQUEST] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 "\n", _objectId, _segmentId);
}
