#include "segmenttransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
#include "../../client/client.hh"
extern Client* client;
#endif
SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(Communicator* communicator) :
		Message(communicator) {
	_threadPoolSize = 10;
}

SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
}

void SegmentTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentTransferEndRequestPro segmentTransferEndRequestPro;
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

	_segmentId = segmentTransferEndRequestPro.segmentid();

}

void SegmentTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentEndProcessor (_msgHeader.requestId, _sockfd, _segmentId);
#endif

#ifdef COMPILE_FOR_CLIENT
	debug ("Start Processor for segment ID = %" PRIu64 "\n", _segmentId);
	client->putSegmentEndProcessor (_msgHeader.requestId, _sockfd, _segmentId);
	debug ("End Processor for segment ID = %" PRIu64 "\n", _segmentId);
#endif
}

void SegmentTransferEndRequestMsg::printProtocol() {
	debug("[SEGMENT_TRANSFER_END_REQUEST] Segment ID = %" PRIu64 "\n", _segmentId);
}
