#include <iostream>
using namespace std;

#include "savesegmentlistrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

SaveSegmentListRequestMsg::SaveSegmentListRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

SaveSegmentListRequestMsg::SaveSegmentListRequestMsg(Communicator* communicator,
		uint32_t sockfd, uint32_t clientId, uint32_t fileId, const vector<uint64_t> &segmentList) :
		Message(communicator) {

	_sockfd = sockfd;
	_clientId = clientId;
	_fileId = fileId;
	_segmentList = segmentList;
}

void SaveSegmentListRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SaveSegmentListRequestPro saveSegmentListRequestPro;
	saveSegmentListRequestPro.set_clientid(_clientId);
	saveSegmentListRequestPro.set_fileid(_fileId);

	for (auto segmentId : _segmentList) {
		saveSegmentListRequestPro.add_segmentlist(segmentId);
	}

	if (!saveSegmentListRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(SAVE_SEGMENT_LIST_REQUEST);
	setProtocolMsg(serializedString);

}

void SaveSegmentListRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SaveSegmentListRequestPro saveSegmentListRequestPro;
	saveSegmentListRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = saveSegmentListRequestPro.clientid();
	_fileId = saveSegmentListRequestPro.fileid();

	for (int i = 0; i < saveSegmentListRequestPro.segmentlist_size(); ++i) {
		_segmentList.push_back(saveSegmentListRequestPro.segmentlist(i));
	}
}

void SaveSegmentListRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->saveSegmentListProcessor (_msgHeader.requestId, _sockfd, _clientId, _fileId, _segmentList);
#endif
}

void SaveSegmentListRequestMsg::printProtocol() {
	debug("[SAVE_SEGMENT_LIST_REQUEST] File ID = %" PRIu32 ", Number of Segment %zu\n", _fileId,_segmentList.size());
}
