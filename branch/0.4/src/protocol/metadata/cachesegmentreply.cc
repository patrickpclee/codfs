#include "cachesegmentreply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

CacheSegmentReplyMsg::CacheSegmentReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

CacheSegmentReplyMsg::CacheSegmentReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd,
		uint64_t segmentId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_msgHeader.requestId = requestId;
	_segmentId = segmentId;
}

void CacheSegmentReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::CacheSegmentReplyPro cacheSegmentReplyPro;
	cacheSegmentReplyPro.set_segmentid(_segmentId);

	if (!cacheSegmentReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(CACHE_SEGMENT_REPLY);
	setProtocolMsg(serializedString);

}

void CacheSegmentReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::CacheSegmentReplyPro cacheSegmentReplyPro;
	cacheSegmentReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = cacheSegmentReplyPro.segmentid();

}

void CacheSegmentReplyMsg::doHandle() {
#ifdef COMPILE_FOR_MDS

#endif
}

void CacheSegmentReplyMsg::printProtocol() {
	debug(
			"[CACHE_SEGMENT_REPLY] Segment ID = %" PRIu64 "\n",
			_segmentId);
}
