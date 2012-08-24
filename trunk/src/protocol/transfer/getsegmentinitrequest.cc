#include "getsegmentinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

GetSegmentInitRequestMsg::GetSegmentInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentInitRequestMsg::GetSegmentInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t objectId, uint32_t segmentId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void GetSegmentInitRequestMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::GetSegmentInitRequestPro getSegmentInitRequestPro;
	getSegmentInitRequestPro.set_objectid(_objectId);
	getSegmentInitRequestPro.set_segmentid(_segmentId);

	if (!getSegmentInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_SEGMENT_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetSegmentInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentInitRequestPro getSegmentInitRequestPro;
	getSegmentInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getSegmentInitRequestPro.objectid();
	_segmentId = getSegmentInitRequestPro.segmentid();
}

void GetSegmentInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getSegmentRequestProcessor (_msgHeader.requestId, _sockfd, _objectId, _segmentId);
#endif
}

void GetSegmentInitRequestMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_INIT] Object ID = %" PRIu64 ", Segment ID = %" PRIu32 "\n",
			_objectId, _segmentId);
}

/*
void GetSegmentInitRequestMsg::setSegmentSize(uint32_t segmentSize) {
	_segmentSize = segmentSize;
}

uint32_t GetSegmentInitRequestMsg::getSegmentSize() {
	return _segmentSize;
}

void GetSegmentInitRequestMsg::setChunkCount(uint32_t chunkCount) {
	_chunkCount = chunkCount;
}

uint32_t GetSegmentInitRequestMsg::getChunkCount() {
	return _chunkCount;
}
*/
