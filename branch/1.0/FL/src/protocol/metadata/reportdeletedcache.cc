#include <iostream>
using namespace std;
#include "reportdeletedcache.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

ReportDeletedCacheMsg::ReportDeletedCacheMsg(Communicator* communicator) :
		Message(communicator) {

}

ReportDeletedCacheMsg::ReportDeletedCacheMsg(Communicator* communicator,
		uint32_t dstSockfd, list<uint64_t> segmentIdList, uint32_t osdId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_segmentIdList = segmentIdList;
	_osdId = osdId;
}

void ReportDeletedCacheMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ReportDeletedCachePro reportDeletedCachePro;

	for (uint64_t segmentId : _segmentIdList) {
		reportDeletedCachePro.add_segmentidlist(segmentId);
	}

	reportDeletedCachePro.set_osdid(_osdId);

	if (!reportDeletedCachePro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(REPORT_DELETED_CACHE);
	setProtocolMsg(serializedString);

}

void ReportDeletedCacheMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ReportDeletedCachePro reportDeletedCachePro;
	reportDeletedCachePro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = reportDeletedCachePro.osdid();
	for (int i = 0; i < reportDeletedCachePro.segmentidlist().size(); ++i) {
		_segmentIdList.push_back(reportDeletedCachePro.segmentidlist(i));
	}

}

void ReportDeletedCacheMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->reportDeleteCacheProcessor(_msgHeader.requestId, _sockfd,
			_segmentIdList, _osdId);
#endif
}

void ReportDeletedCacheMsg::printProtocol() {
	debug(
			"[REPORT_DELETED_CACHE] Segment ID List Osd ID = %" PRIu32 " Size = %zu\n",
			_osdId, _segmentIdList.size());
}
