#include "osdstatupdaterequestmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

OsdStatUpdateRequestMsg::OsdStatUpdateRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

OsdStatUpdateRequestMsg::OsdStatUpdateRequestMsg(Communicator* communicator, uint32_t osdSockfd) :
		Message(communicator) {

	_sockfd = osdSockfd;
}

void OsdStatUpdateRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::OsdStatUpdateRequestPro osdStatUpdateRequestPro;

	if (!osdStatUpdateRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (OSDSTAT_UPDATE_REQUEST);
	setProtocolMsg(serializedString);

}

void OsdStatUpdateRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::OsdStatUpdateRequestPro osdStatUpdateRequestPro;
	osdStatUpdateRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

}

void OsdStatUpdateRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->OsdStatUpdateRequestProcessor(_msgHeader.requestId, _sockfd);
#endif
}

void OsdStatUpdateRequestMsg::printProtocol() {
}
