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
SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(
		Communicator* communicator) :
		Message(communicator) {
	_threadPoolSize = 4;
}

SegmentTransferEndRequestMsg::SegmentTransferEndRequestMsg(
		Communicator* communicator, uint32_t osdSockfd, uint64_t segmentId,
		DataMsgType dataMsgType, string updateKey,
		vector<offset_length_t> offsetlength) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_dataMsgType = dataMsgType;
	_offsetLength = offsetlength;
	_updateKey = updateKey;
}

void SegmentTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SegmentTransferEndRequestPro segmentTransferEndRequestPro;
	segmentTransferEndRequestPro.set_segmentid(_segmentId);
	segmentTransferEndRequestPro.set_datamsgtype(
			(ncvfs::DataMsgPro_DataMsgType) _dataMsgType);
	segmentTransferEndRequestPro.set_updatekey(_updateKey);

	vector<offset_length_t>::iterator it;
	for (it = _offsetLength.begin(); it < _offsetLength.end(); ++it) {
		ncvfs::OffsetLengthPro* offsetLengthPro =
				segmentTransferEndRequestPro.add_offsetlength();
		offsetLengthPro->set_offset((*it).first);
		offsetLengthPro->set_length((*it).second);
	}

	if (!segmentTransferEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SEGMENT_TRANSFER_END_REQUEST);
	setProtocolMsg(serializedString);

}

void SegmentTransferEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SegmentTransferEndRequestPro segmentTransferEndRequestPro;
	segmentTransferEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = segmentTransferEndRequestPro.segmentid();
	_dataMsgType = (DataMsgType) segmentTransferEndRequestPro.datamsgtype();
	_updateKey = segmentTransferEndRequestPro.updatekey();

	for (int i = 0; i < segmentTransferEndRequestPro.offsetlength_size(); ++i) {
		offset_length_t tempOffsetLength;

		uint32_t offset = segmentTransferEndRequestPro.offsetlength(i).offset();
		uint32_t length = segmentTransferEndRequestPro.offsetlength(i).length();
		tempOffsetLength = make_pair(offset, length);

		_offsetLength.push_back(tempOffsetLength);
	}

}

void SegmentTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentEndProcessor(_msgHeader.requestId, _sockfd, _segmentId,
			_dataMsgType, _updateKey, _offsetLength);
#endif

#ifdef COMPILE_FOR_CLIENT
	debug("Start Processor for segment ID = %" PRIu64 "\n", _segmentId);
	client->putSegmentEndProcessor(_msgHeader.requestId, _sockfd, _segmentId);
	debug("End Processor for segment ID = %" PRIu64 "\n", _segmentId);
#endif
}

void SegmentTransferEndRequestMsg::printProtocol() {
	debug("[SEGMENT_TRANSFER_END_REQUEST] Segment ID = %" PRIu64 "\n",
			_segmentId);
}
