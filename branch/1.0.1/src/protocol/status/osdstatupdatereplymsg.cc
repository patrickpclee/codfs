#include "osdstatupdatereplymsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

OsdStatUpdateReplyMsg::OsdStatUpdateReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

OsdStatUpdateReplyMsg::OsdStatUpdateReplyMsg(Communicator* communicator, uint32_t osdSockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_osdId = osdId;
	_capacity = capacity;
	_loading = loading;
	
}

void OsdStatUpdateReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OsdStatUpdateReplyPro osdStatUpdateReplyPro;
	osdStatUpdateReplyPro.set_osdid(_osdId);
	osdStatUpdateReplyPro.set_osdcapacity(_capacity);
	osdStatUpdateReplyPro.set_osdloading(_loading);

	if (!osdStatUpdateReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OSDSTAT_UPDATE_REPLY);
	setProtocolMsg(serializedString);

}

void OsdStatUpdateReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OsdStatUpdateReplyPro osdStatUpdateReplyPro;
	osdStatUpdateReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = osdStatUpdateReplyPro.osdid();
	_capacity = osdStatUpdateReplyPro.osdcapacity();
	_loading = osdStatUpdateReplyPro.osdloading();

}

void OsdStatUpdateReplyMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->OsdStatUpdateReplyProcessor(_msgHeader.requestId, _sockfd, _osdId, _capacity, _loading);
#endif
}

void OsdStatUpdateReplyMsg::printProtocol() {
	debug("[OSDSTAT_UPDATE_REPLY] Osd ID = %" PRIu32 ", capacity = %" PRIu64 ", loading = %" PRIu32 "\n",
			_osdId, _capacity, _loading);
}
