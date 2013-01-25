#include "precachesegmentrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

PrecacheSegmentRequestMsg::PrecacheSegmentRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PrecacheSegmentRequestMsg::PrecacheSegmentRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint32_t clientId, uint64_t segmentId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_clientId = clientId;
	_segmentId = segmentId;
}

void PrecacheSegmentRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PrecacheSegmentRequestPro precacheSegmentRequestPro;
	precacheSegmentRequestPro.set_segmentid(_segmentId);
	precacheSegmentRequestPro.set_clientid(_clientId);

	if (!precacheSegmentRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PRECACHE_SEGMENT_REQUEST);
	setProtocolMsg(serializedString);

}

void PrecacheSegmentRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PrecacheSegmentRequestPro precacheSegmentRequestPro;
	precacheSegmentRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = precacheSegmentRequestPro.clientid();
	_segmentId = precacheSegmentRequestPro.segmentid();

}

void PrecacheSegmentRequestMsg::doHandle() {
/*
#ifdef COMPILE_FOR_MDS
	mds->precacheSegmentRequestProcessor (_msgHeader.requestId, _sockfd, _clientId, _segmentId);
#endif
*/
}

void PrecacheSegmentRequestMsg::printProtocol() {
	debug("[PRECACHE_SEGMENT_REQUEST] Client ID = %" PRIu32 " Segment ID = %" PRIu64 "\n", _clientId, _segmentId);
}
