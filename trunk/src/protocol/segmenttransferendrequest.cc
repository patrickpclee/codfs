#include "segmenttransferendrequest.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../osd/osd.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void SegmentTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentTransferEndRequestPro segmentTransferEndRequestPro;
	segmentTransferEndRequestPro.set_objectid(_objectId);
	segmentTransferEndRequestPro.set_segmentid(_segmentId);

	if (!segmentTransferEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (SEGMENT_TRANSFER_END_REQUEST);
	setProtocolMsg(serializedString);

}

void SegmentTransferEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SegmentTransferEndRequestPro segmentTransferEndRequestPro;
	segmentTransferEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = segmentTransferEndRequestPro.objectid();
	_segmentId = segmentTransferEndRequestPro.segmentid();

}

void SegmentTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentEndProcessor (_msgHeader.requestId, _sockfd, _objectId, _segmentId);
#endif
}

void SegmentTransferEndRequestMsg::printProtocol() {
	debug("[SEGMENT_TRANSFER_END_REQUEST] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 "\n", _objectId, _segmentId);
}
