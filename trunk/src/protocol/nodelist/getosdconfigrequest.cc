#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "getosdconfigrequest.hh"

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif


GetOsdConfigRequestMsg::GetOsdConfigRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetOsdConfigRequestMsg::GetOsdConfigRequestMsg(Communicator* communicator, uint32_t Sockfd) :
		Message(communicator) {

	_sockfd = Sockfd;
}

void GetOsdConfigRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetOsdConfigRequestPro getOsdConfigRequestPro;

	if (!getOsdConfigRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OSD_CONFIG_REQUEST);
	setProtocolMsg(serializedString);

}

void GetOsdConfigRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdConfigRequestPro getOsdConfigRequestPro;
	getOsdConfigRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);


}

void GetOsdConfigRequestMsg::setParam(uint32_t osdId, uint32_t port,
		uint32_t segmentCapacity, uint32_t objectCache, string segmentFolder, string objectCacheFolder){
	_osdId = osdId;
	_servePort = port;
	_segmentCapacity = segmentCapacity;
	_objectCacheCapacity = objectCache;
	_segmentFolder = segmentFolder;
	_objectCacheFolder = objectCacheFolder;
}

void GetOsdConfigRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MONITOR
	monitor->getOsdConfigProcessor (_msgHeader.requestId, _sockfd);
#endif
}

void GetOsdConfigRequestMsg::printProtocol() {
}

