#include <iostream>
using namespace std;
#include "getosdconfigrequest.hh"
#include "getosdconfigreply.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MONITOR
#include "../monitor/monitor.hh"
extern Monitor* monitor;
#endif

GetOsdConfigReplyMsg::GetOsdConfigReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

GetOsdConfigReplyMsg::GetOsdConfigReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint32_t osdId, uint16_t port,
		uint32_t segmentCapacity, uint32_t objectCache, string segmentFolder, string objectCacheFolder) :
		Message(communicator) {
	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_osdId = osdId;
	_servePort = port;
	_segmentCapacity = segmentCapacity;
	_objectCacheCapacity = objectCache;
	_segmentFolder = segmentFolder;
	_objectCacheFolder = objectCacheFolder;

}

void GetOsdConfigReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetOsdConfigReplyPro getOsdConfigReplyPro;

	getOsdConfigReplyPro.set_osdid(_osdId);
	getOsdConfigReplyPro.set_port(_servePort);
	getOsdConfigReplyPro.set_segmentcapacity(_segmentCapacity);
	getOsdConfigReplyPro.set_objectcachecapacity(_objectCacheCapacity);
	getOsdConfigReplyPro.set_segmentfolder(_segmentFolder);
	getOsdConfigReplyPro.set_objectcachefolder(_objectCacheFolder);

	if (!getOsdConfigReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_OSD_CONFIG_REPLY);
	setProtocolMsg(serializedString);

}

void GetOsdConfigReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetOsdConfigReplyPro getOsdConfigReplyPro;
	getOsdConfigReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = getOsdConfigReplyPro.osdid();
	_servePort = getOsdConfigReplyPro.port();
	_segmentCapacity = getOsdConfigReplyPro.segmentcapacity();
	_objectCacheCapacity = getOsdConfigReplyPro.objectcachecapacity();
	_segmentFolder = getOsdConfigReplyPro.segmentfolder();
	_objectCacheFolder = getOsdConfigReplyPro.objectcachefolder();
	return;
}

void GetOsdConfigReplyMsg::doHandle() {
	GetOsdConfigRequestMsg* getOsdConfigRequestMsg =
			(GetOsdConfigRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getOsdConfigRequestMsg->setParam(_osdId,_servePort,_segmentCapacity, _objectCacheCapacity, _segmentFolder, _objectCacheFolder)
	getOsdConfigRequestMsg->setStatus(READY);
}

void GetOsdConfigReplyMsg::printProtocol() {
	debug("%s\n", "[GET_OSD_CONFIG_REPLY] GOT.");
}
