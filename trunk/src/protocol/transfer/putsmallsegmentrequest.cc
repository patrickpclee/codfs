#include "putsmallsegmentrequest.hh"
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

PutSmallSegmentRequestMsg::PutSmallSegmentRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSmallSegmentRequestMsg::PutSmallSegmentRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t segmentSize,
		uint32_t bufferSize, CodingScheme codingScheme,
		const string &codingSetting, vector<offset_length_t> offsetLength,
		string updateKey) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
	_updateKey = updateKey;		// DEFAULT: ""
	_bufferSize = bufferSize;
	_offsetLength = offsetLength;
}

void PutSmallSegmentRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSmallSegmentRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.set_segmentid(_segmentId);
	putSegmentInitRequestPro.set_segmentsize(_segmentSize);
	putSegmentInitRequestPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	putSegmentInitRequestPro.set_codingsetting(_codingSetting);
	putSegmentInitRequestPro.set_updatekey(_updateKey);
	putSegmentInitRequestPro.set_buffersize(_bufferSize);

    vector<offset_length_t>::iterator it;
    for (it = _offsetLength.begin(); it < _offsetLength.end(); ++it) {
        ncvfs::OffsetLengthPro* offsetLengthPro =
                putSegmentInitRequestPro.add_offsetlength();
        offsetLengthPro->set_offset((*it).first);
        offsetLengthPro->set_length((*it).second);
    }

	if (!putSegmentInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_SMALL_SEGMENT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutSmallSegmentRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSmallSegmentRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = putSegmentInitRequestPro.segmentid();
	_segmentSize = putSegmentInitRequestPro.segmentsize();
	_codingScheme = (CodingScheme) putSegmentInitRequestPro.codingscheme();
	_codingSetting = putSegmentInitRequestPro.codingsetting();
	_updateKey = putSegmentInitRequestPro.updatekey();
	_bufferSize = putSegmentInitRequestPro.buffersize();

    for (int i = 0; i < putSegmentInitRequestPro.offsetlength_size(); ++i) {
        offset_length_t tempOffsetLength;

        uint32_t offset = putSegmentInitRequestPro.offsetlength(i).offset();
        uint32_t length = putSegmentInitRequestPro.offsetlength(i).length();
        tempOffsetLength = make_pair(offset, length);

        _offsetLength.push_back(tempOffsetLength);
    }
}

void PutSmallSegmentRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
    _dataMsgType = osd->putSegmentInitProcessor(_msgHeader.requestId, _sockfd,
            _segmentId, _segmentSize, _bufferSize, 1, _codingScheme,
            _codingSetting, _updateKey, true);
    osd->putSegmentDataProcessor(_msgHeader.requestId, _sockfd, _segmentId, 0,
            _bufferSize, _dataMsgType, _updateKey, _payload);
    osd->putSegmentEndProcessor(_msgHeader.requestId, _sockfd, _segmentId,
            _dataMsgType, _updateKey, _offsetLength, true);
#endif

#ifdef COMPILE_FOR_CLIENT
    client->putSegmentInitProcessor(_msgHeader.requestId, _sockfd, _segmentId,
            _segmentSize, _bufferSize, 1, true);
    client->SegmentDataProcessor(_msgHeader.requestId, _sockfd, _segmentId, 0,
            _bufferSize, _payload);
    client->putSegmentEndProcessor(_msgHeader.requestId, _sockfd, _segmentId,
            true);
#endif
}

void PutSmallSegmentRequestMsg::printProtocol() {
	debug(
			"[PUT_SEGMENT_INIT_REQUEST] Segment ID = %" PRIu64 ", SegLength = %" PRIu32 ", BufLength = %" PRIu32 ", CodingScheme = %" PRIu32 " CodingSetting = %s\n",
			_segmentId, _segmentSize, _bufferSize,  _codingScheme,
			_codingSetting.c_str());
}

void PutSmallSegmentRequestMsg::setDataMsgType (DataMsgType dataMsgType) {
    _dataMsgType = dataMsgType;
}

DataMsgType PutSmallSegmentRequestMsg::getDataMsgType () {
    return _dataMsgType;
}

