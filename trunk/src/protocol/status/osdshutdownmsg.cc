#include "osdshutdownmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

OsdShutdownMsg::OsdShutdownMsg(Communicator* communicator) :
		Message(communicator) {

}

OsdShutdownMsg::OsdShutdownMsg(Communicator* communicator, uint32_t osdSockfd,
		uint32_t osdId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_osdId = osdId;
}

void OsdShutdownMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OsdShutdownPro osdShutdownPro;
	osdShutdownPro.set_osdid(_osdId);

	if (!osdShutdownPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OSD_SHUTDOWN);
	setProtocolMsg(serializedString);

}

void OsdShutdownMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OsdShutdownPro osdShutdownPro;
	osdShutdownPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = osdShutdownPro.osdid();

}

void OsdShutdownMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->OsdShutdownProcessor(_msgHeader.requestId, _sockfd, _osdId);
#endif
}

void OsdShutdownMsg::printProtocol() {
	debug("[OSD_SHUTDOWN] Osd ID = %" PRIu32 "\n", _osdId);
}
