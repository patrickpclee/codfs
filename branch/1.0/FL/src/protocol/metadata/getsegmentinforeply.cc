#include <iostream>
#include "getsegmentinforequest.hh"
#include "getsegmentinforeply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

GetSegmentInfoReplyMsg::GetSegmentInfoReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetSegmentInfoReplyMsg::GetSegmentInfoReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t segmentId,
		uint32_t segmentSize, const vector<uint32_t> &nodeList,
		CodingScheme codingScheme, const string &codingSetting) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;

	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_nodeList = nodeList;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;

}

void GetSegmentInfoReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetSegmentInfoReplyPro getSegmentInfoReplyPro;
	getSegmentInfoReplyPro.set_segmentid(_segmentId);
	getSegmentInfoReplyPro.set_segmentsize(_segmentSize);
	getSegmentInfoReplyPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	getSegmentInfoReplyPro.set_codingsetting(_codingSetting);

	for (auto nodeID : _nodeList) {
		getSegmentInfoReplyPro.add_nodelist(nodeID);
	}

	if (!getSegmentInfoReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_SEGMENT_INFO_REPLY);
	setProtocolMsg(serializedString);

}

void GetSegmentInfoReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetSegmentInfoReplyPro getSegmentInfoReplyPro;
	getSegmentInfoReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getSegmentInfoReplyPro.segmentid();
	_segmentSize = getSegmentInfoReplyPro.segmentsize();
	_codingScheme = (CodingScheme) getSegmentInfoReplyPro.codingscheme();
	_codingSetting = getSegmentInfoReplyPro.codingsetting();

	for (int i = 0; i < getSegmentInfoReplyPro.nodelist_size(); i++) {
		_nodeList.push_back(getSegmentInfoReplyPro.nodelist(i));
	}

}

void GetSegmentInfoReplyMsg::doHandle() {
	GetSegmentInfoRequestMsg* getSegmentInfoRequestMsg =
			(GetSegmentInfoRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getSegmentInfoRequestMsg->setCodingScheme(_codingScheme);
	getSegmentInfoRequestMsg->setCodingSetting(_codingSetting);
	getSegmentInfoRequestMsg->setNodeList(_nodeList);
	getSegmentInfoRequestMsg->setSegmentSize(_segmentSize);
	getSegmentInfoRequestMsg->setStatus(READY);
}

void GetSegmentInfoReplyMsg::printProtocol() {
	debug(
			"[GET_SEGMENT_INFO_REPLY] Segment ID = %" PRIu64 " Size = %" PRIu32 " CodingScheme = %d Setting = %s\n",
			_segmentId, _segmentSize, (int)_codingScheme, _codingSetting.c_str());
}
