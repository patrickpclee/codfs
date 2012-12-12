#include "getsegmentrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

GetSegmentRequestMsg::GetSegmentRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentRequestMsg::GetSegmentRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint64_t segmentId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_segmentId = segmentId;
}

void GetSegmentRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentRequestPro getSegmentRequestPro;
	getSegmentRequestPro.set_segmentid(_segmentId);

	if (!getSegmentRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_SEGMENT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetSegmentRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentRequestPro getSegmentRequestPro;
	getSegmentRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getSegmentRequestPro.segmentid();
	_segmentSize = getSegmentRequestPro.segmentsize();
	_chunkCount = getSegmentRequestPro.chunkcount();

}

void GetSegmentRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getSegmentRequestProcessor (_msgHeader.requestId, _sockfd, _segmentId);
#endif
}

void GetSegmentRequestMsg::setSegmentSize(uint32_t segmentSize){
	_segmentSize = segmentSize;
}

void GetSegmentRequestMsg::setChunkCount(uint32_t chunkCount){
	_chunkCount = chunkCount;
}

uint32_t GetSegmentRequestMsg::getSegmentSize(){
	return _segmentSize;
}

uint32_t GetSegmentRequestMsg::getChunkCount(){
	return _chunkCount;
}

uint32_t GetSegmentRequestMsg::getRequestId(){
	return _msgHeader.requestId;
}

void GetSegmentRequestMsg::printProtocol() {
	debug("[GET_SEGMENT_REQUEST] Segment ID = %" PRIu64 "\n", _segmentId);
}
