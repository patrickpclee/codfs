#include "getsegmentreadymsg.hh"
#include "../../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../osd/osd.hh"

GetSegmentReadyMsg::GetSegmentReadyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentReadyMsg::GetSegmentReadyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t objectId, uint32_t
		segmentId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_objectId = objectId;
	_segmentId = segmentId;
}

void GetSegmentReadyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentReadyPro getSegmentReadyPro;
	getSegmentReadyPro.set_objectid(_objectId);
	getSegmentReadyPro.set_segmentid(_segmentId);

	if (!getSegmentReadyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_SEGMENT_READY);
	setProtocolMsg(serializedString);

}

void GetSegmentReadyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentReadyPro getSegmentReadyPro;
	getSegmentReadyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getSegmentReadyPro.objectid();
	_segmentId = getSegmentReadyPro.segmentid();

}

void GetSegmentReadyMsg::doHandle() {
}

void GetSegmentReadyMsg::printProtocol() {
	debug("[GET_SEGMENT_READY] Object ID = %" PRIu64 " SEGMENT ID = %" PRIu32 "\n", _objectId, _segmentId);
}
