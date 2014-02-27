#include "cachesegmentrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

CacheSegmentRequestMsg::CacheSegmentRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

CacheSegmentRequestMsg::CacheSegmentRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint64_t segmentId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_segmentId = segmentId;
}

void CacheSegmentRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::CacheSegmentRequestPro cacheSegmentRequestPro;
	cacheSegmentRequestPro.set_segmentid(_segmentId);

	if (!cacheSegmentRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (CACHE_SEGMENT_REQUEST);
	setProtocolMsg(serializedString);

}

void CacheSegmentRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::CacheSegmentRequestPro cacheSegmentRequestPro;
	cacheSegmentRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = cacheSegmentRequestPro.segmentid();

}

void CacheSegmentRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getSegmentRequestProcessor (_msgHeader.requestId, _sockfd, _segmentId, true); // local retrieve
#endif
}

uint32_t CacheSegmentRequestMsg::getRequestId(){
	return _msgHeader.requestId;
}

void CacheSegmentRequestMsg::printProtocol() {
	debug("[CACHE_SEGMENT_REQUEST] Segment ID = %" PRIu64 "\n", _segmentId);
}
