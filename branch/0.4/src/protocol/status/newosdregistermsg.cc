#include <iostream>
using namespace std;
#include "newosdregistermsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

NewOsdRegisterMsg::NewOsdRegisterMsg(Communicator* communicator) :
		Message(communicator) {

}

NewOsdRegisterMsg::NewOsdRegisterMsg(Communicator* communicator, uint32_t sockfd,
		uint32_t osdId, uint32_t osdIp, uint32_t osdPort) :
		Message(communicator) {

	_sockfd = sockfd;
	_osdId = osdId;
	_osdIp = osdIp;
	_osdPort = osdPort;
}

void NewOsdRegisterMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::NewOsdRegisterPro newOsdRegisterPro;
	newOsdRegisterPro.set_osdid(_osdId);
	newOsdRegisterPro.set_osdip(_osdIp);
	newOsdRegisterPro.set_osdport(_osdPort);

	if (!newOsdRegisterPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (NEW_OSD_REGISTER);
	setProtocolMsg(serializedString);

}

void NewOsdRegisterMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::NewOsdRegisterPro newOsdRegisterPro;
	newOsdRegisterPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_osdId = newOsdRegisterPro.osdid();
	_osdIp = newOsdRegisterPro.osdip();
	_osdPort = newOsdRegisterPro.osdport();

}

void NewOsdRegisterMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->NewOsdRegisterProcessor(_msgHeader.requestId, _sockfd, _osdId, 
		_osdIp, _osdPort);
#endif
}

void NewOsdRegisterMsg::printProtocol() {
	debug("[NEW_OSD_REGISTER] Osd ID = %" PRIu32 "\n", _osdId);
}
