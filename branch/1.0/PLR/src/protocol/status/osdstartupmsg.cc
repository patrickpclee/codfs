#include <iostream>
using namespace std;
#include "osdstartupmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../../monitor/monitor.hh"
extern Monitor* monitor;
#endif

OsdStartupMsg::OsdStartupMsg(Communicator* communicator) :
		Message(communicator) {

}

OsdStartupMsg::OsdStartupMsg(Communicator* communicator, uint32_t osdSockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_osdId = osdId;
	_capacity = capacity;
	_loading = loading;
}

OsdStartupMsg::OsdStartupMsg(Communicator* communicator, uint32_t osdSockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_osdId = osdId;
	_capacity = capacity;
	_loading = loading;
	_osdIp = ip;
	_osdPort = port;
}

void OsdStartupMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OsdStartupPro osdStartupPro;
	osdStartupPro.set_osdid(_osdId);
	osdStartupPro.set_osdcapacity(_capacity);
	osdStartupPro.set_osdloading(_loading);
	osdStartupPro.set_osdip(_osdIp);
	osdStartupPro.set_osdport(_osdPort);

	if (!osdStartupPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OSD_STARTUP);
	setProtocolMsg(serializedString);

}

void OsdStartupMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OsdStartupPro osdStartupPro;
	osdStartupPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = osdStartupPro.osdid();
	_capacity = osdStartupPro.osdcapacity();
	_loading = osdStartupPro.osdloading();
	_osdIp = osdStartupPro.osdip();
	_osdPort = osdStartupPro.osdport();

}

void OsdStartupMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->OsdStartupProcessor(_msgHeader.requestId, _sockfd, _osdId, 
		_capacity, _loading, _osdIp, _osdPort);
#endif
}

void OsdStartupMsg::printProtocol() {
	debug("[OSD_STARTUP] Osd ID = %" PRIu32 ", capacity = %" PRIu32 ", loading = %" PRIu32 "\n",
			_osdId, _capacity, _loading);
}
