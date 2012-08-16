#include "segmentdatamsg.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../osd/osd.hh"
#include "../client/client.hh"

#ifdef COMPILE_FOR_OSD
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
extern Client* client;
#endif

SegmentDataMsg::SegmentDataMsg(Communicator* communicator) :
		Message(communicator) {

}

SegmentDataMsg::SegmentDataMsg(Communicator* communicator, uint32_t osdSockfd,
		uint64_t objectId, uint32_t segmentId, uint64_t offset, uint32_t length) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
	_offset = offset;
	_length = length;
}

void SegmentDataMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentDataPro segmentDataPro;
	segmentDataPro.set_objectid(_objectId);
	segmentDataPro.set_segmentid(_segmentId);
	segmentDataPro.set_offset(_offset);
	segmentDataPro.set_length(_length);

	if (!segmentDataPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SEGMENT_DATA);
	setProtocolMsg(serializedString);

}

void SegmentDataMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SegmentDataPro segmentDataPro;
	segmentDataPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = segmentDataPro.objectid();
	_segmentId = segmentDataPro.segmentid();
	_offset = segmentDataPro.offset();
	_length = segmentDataPro.length();

}

void SegmentDataMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentDataProcessor(_msgHeader.requestId, _sockfd, _objectId, _segmentId, _offset, _length, _payload);
#endif
}

void SegmentDataMsg::printProtocol() {
	debug("[SEGMENT_DATA] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 ", offset = %" PRIu64 ", length = %" PRIu32 "\n",
			_objectId, _segmentId, _offset, _length);
}
