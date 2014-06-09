#include "putsegmentinitrequest.hh"
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

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t segmentSize,
		uint32_t bufferSize, uint32_t chunkCount, CodingScheme codingScheme,
		const string &codingSetting, string updateKey) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_bufferSize = bufferSize;
	_chunkCount = chunkCount;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
	_updateKey = updateKey;		// DEFAULT: ""
}

void PutSegmentInitRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.set_segmentid(_segmentId);
	putSegmentInitRequestPro.set_segmentsize(_segmentSize);
	putSegmentInitRequestPro.set_buffersize(_bufferSize);
	putSegmentInitRequestPro.set_chunkcount(_chunkCount);
	putSegmentInitRequestPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	putSegmentInitRequestPro.set_codingsetting(_codingSetting);
	putSegmentInitRequestPro.set_updatekey(_updateKey);

	if (!putSegmentInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_SEGMENT_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutSegmentInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentInitRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = putSegmentInitRequestPro.segmentid();
	_segmentSize = putSegmentInitRequestPro.segmentsize();
	_bufferSize = putSegmentInitRequestPro.buffersize();
	_chunkCount = putSegmentInitRequestPro.chunkcount();
	_codingScheme = (CodingScheme) putSegmentInitRequestPro.codingscheme();
	_codingSetting = putSegmentInitRequestPro.codingsetting();
	_updateKey = putSegmentInitRequestPro.updatekey();

}

void PutSegmentInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentInitProcessor(_msgHeader.requestId, _sockfd, _segmentId,
			_segmentSize, _bufferSize, _chunkCount, _codingScheme, _codingSetting,
			_updateKey);
#endif

#ifdef COMPILE_FOR_CLIENT
	client->putSegmentInitProcessor(_msgHeader.requestId, _sockfd, _segmentId,
			_segmentSize, _bufferSize, _chunkCount);
#endif
}

void PutSegmentInitRequestMsg::printProtocol() {
	debug(
			"[PUT_SEGMENT_INIT_REQUEST] Segment ID = %" PRIu64 ", SegLength = %" PRIu32 ", BufLength = %" PRIu32 ", Count = %" PRIu32 " CodingScheme = %" PRIu32 " CodingSetting = %s \n",
			_segmentId, _segmentSize, _bufferSize,  _chunkCount, _codingScheme,
			_codingSetting.c_str());
}

void PutSegmentInitRequestMsg::setDataMsgType (DataMsgType dataMsgType) {
    _dataMsgType = dataMsgType;
}

DataMsgType PutSegmentInitRequestMsg::getDataMsgType () {
    return _dataMsgType;
}

