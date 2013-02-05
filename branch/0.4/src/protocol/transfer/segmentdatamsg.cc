#include "segmentdatamsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
#include "../client/client.hh"
extern Client* client;
#endif

SegmentDataMsg::SegmentDataMsg(Communicator* communicator) :
		Message(communicator) {

}

SegmentDataMsg::SegmentDataMsg(Communicator* communicator, uint32_t osdSockfd,
		uint64_t segmentId, uint64_t offset, uint32_t length) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_offset = offset;
	_length = length;
}

void SegmentDataMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentDataPro segmentDataPro;
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

	_segmentId = segmentDataPro.segmentid();
	_offset = segmentDataPro.offset();
	_length = segmentDataPro.length();

}

void SegmentDataMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentDataProcessor(_msgHeader.requestId, _sockfd, _segmentId, _offset, _length, _payload);
#endif
#ifdef COMPILE_FOR_CLIENT
	client->SegmentDataProcessor(_msgHeader.requestId, _sockfd, _segmentId, _offset, _length, _payload);
#endif
}

void SegmentDataMsg::printProtocol() {
	debug("[SEGMENT_DATA] Segment ID = %" PRIu64 ", offset = %" PRIu64 ", length = %" PRIu32 "\n",
			_segmentId, _offset, _length);
}
