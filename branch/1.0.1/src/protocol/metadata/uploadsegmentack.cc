#include <iostream>

#include "uploadsegmentack.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

UploadSegmentAckMsg::UploadSegmentAckMsg(Communicator* communicator) :
		Message(communicator) {
}

UploadSegmentAckMsg::UploadSegmentAckMsg(Communicator* communicator,
		uint32_t sockfd, uint64_t segmentId, uint32_t segmentSize, CodingScheme codingScheme,
		const string &codingSetting, const vector<uint32_t> &nodeList) :
		Message(communicator) {
	_sockfd = sockfd;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
	_nodeList = nodeList;
}

void UploadSegmentAckMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::UploadSegmentAckPro uploadSegmentAckPro;

	uploadSegmentAckPro.set_segmentid((long long int) _segmentId);
	uploadSegmentAckPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	uploadSegmentAckPro.set_codingsetting(_codingSetting);
	uploadSegmentAckPro.set_segmentsize(_segmentSize);

	vector<uint32_t>::iterator it;

	for (it = _nodeList.begin(); it < _nodeList.end(); ++it) {
		uploadSegmentAckPro.add_nodelist(*it);
	}

	if (!uploadSegmentAckPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(UPLOAD_SEGMENT_ACK);
	setProtocolMsg(serializedString);

	return;
}

void UploadSegmentAckMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadSegmentAckPro uploadSegmentAckPro;
	uploadSegmentAckPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = uploadSegmentAckPro.segmentid();
	_codingScheme = (CodingScheme) uploadSegmentAckPro.codingscheme();
	_codingSetting = uploadSegmentAckPro.codingsetting();
	_segmentSize = uploadSegmentAckPro.segmentsize();

	for (int i = 0; i < uploadSegmentAckPro.nodelist_size(); ++i) {
		_nodeList.push_back(uploadSegmentAckPro.nodelist(i));
	}

	return;
}

void UploadSegmentAckMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->uploadSegmentAckProcessor(_msgHeader.requestId, _sockfd, _segmentId, _segmentSize, _codingScheme, _codingSetting, _nodeList);
#endif
}

void UploadSegmentAckMsg::printProtocol() {
	debug("[UPLOAD_SEGMENT_ACK] Segment ID = %" PRIu64 " Coding Scheme = %d Coding Setting = %s\n", _segmentId, _codingScheme, _codingSetting.c_str());
}
