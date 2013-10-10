#include "getsegmentcodinginforeply.hh"
#include "getsegmentcodinginforequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

GetSegmentCodingInfoReplyMsg::GetSegmentCodingInfoReplyMsg(Communicator* communicator) :
        Message(communicator) {

}

GetSegmentCodingInfoReplyMsg::GetSegmentCodingInfoReplyMsg(
        Communicator* communicator, uint32_t requestId, uint32_t osdSockfd,
        vector<SegmentCodingInfo> segmentCodingInfo) :
        Message(communicator) {

    _msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_segmentCodingInfo = segmentCodingInfo;
}

void GetSegmentCodingInfoReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentCodingInfoReplyPro getSegmentCodingInfoReplyPro;

	vector<SegmentCodingInfo>::iterator it;
	for (it = _segmentCodingInfo.begin(); it < _segmentCodingInfo.end(); ++it) {
		ncvfs::GetSegmentCodingInfoReplyPro_SegmentCodingInfoPro* segmentCodingInfoPro =
				getSegmentCodingInfoReplyPro.add_segmentcodinginfo();
		segmentCodingInfoPro->set_segmentid(it->segmentId);
		segmentCodingInfoPro->set_segmentsize(it->segmentId);
		segmentCodingInfoPro->set_codingscheme(
		        (ncvfs::PutSegmentInitRequestPro_CodingScheme) it->codingScheme);
		segmentCodingInfoPro->set_codingsetting(it->codingSetting);
	}

	if (!getSegmentCodingInfoReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_INFO_REPLY);
	setProtocolMsg(serializedString);

}

void GetSegmentCodingInfoReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentCodingInfoReplyPro getSegmentCodingInfoReplyPro;
	getSegmentCodingInfoReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);


	for (int i = 0; i < getSegmentCodingInfoReplyPro.segmentcodinginfo_size(); ++i) {
	    SegmentCodingInfo temp;

		temp.segmentId = getSegmentCodingInfoReplyPro.segmentcodinginfo(i).segmentid();
		temp.segmentSize = getSegmentCodingInfoReplyPro.segmentcodinginfo(i).segmentsize();
		temp.codingScheme = (CodingScheme)getSegmentCodingInfoReplyPro.segmentcodinginfo(i).codingscheme();
		temp.codingSetting = getSegmentCodingInfoReplyPro.segmentcodinginfo(i).codingsetting();

		_segmentCodingInfo.push_back(temp);
	}

}

void GetSegmentCodingInfoReplyMsg::doHandle() {
    GetSegmentCodingInfoRequestMsg* getSegmentCodingInfoRequestMsg =
            (GetSegmentCodingInfoRequestMsg*) _communicator->popWaitReplyMessage(
                    _msgHeader.requestId);
    getSegmentCodingInfoRequestMsg->setSegmentCodingInfoList(_segmentCodingInfo);
    getSegmentCodingInfoRequestMsg->setStatus(READY);
}

void GetSegmentCodingInfoReplyMsg::printProtocol() {
	debug("[GET_SEGMENT_CODING_INFO_REPLY] Segment ID List size = %zu\n", _segmentCodingInfo.size());
}
