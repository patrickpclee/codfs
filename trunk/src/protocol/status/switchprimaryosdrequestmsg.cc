#include <iostream>
#include "switchprimaryosdrequestmsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

using namespace std;
SwitchPrimaryOsdRequestMsg::SwitchPrimaryOsdRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

SwitchPrimaryOsdRequestMsg::SwitchPrimaryOsdRequestMsg(Communicator* communicator, uint32_t sockfd,
		uint64_t objectId) :
		Message(communicator) {

	_sockfd = sockfd;
	_objectId = objectId;
}

void SwitchPrimaryOsdRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::SwitchPrimaryOsdRequestPro switchPrimaryOsdRequestPro;
	switchPrimaryOsdRequestPro.set_objectid(_objectId);

	if (!switchPrimaryOsdRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (SWITCH_PRIMARY_OSD_REQUEST);
	setProtocolMsg(serializedString);

}

void SwitchPrimaryOsdRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::SwitchPrimaryOsdRequestPro switchPrimaryOsdRequestPro;
	switchPrimaryOsdRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = switchPrimaryOsdRequestPro.objectid();

}

void SwitchPrimaryOsdRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
#endif
}

void SwitchPrimaryOsdRequestMsg::printProtocol() {
	debug("[SWITCH PRIMARY OSD REQUEST] Object ID = %" PRIu64 "\n",
			_objectId);
}

void SwitchPrimaryOsdRequestMsg::setNewPrimaryOsdId(uint32_t id) {
	_newPrimaryOsdId = id;
}

uint32_t SwitchPrimaryOsdRequestMsg::getNewPrimaryOsdId() {
	return _newPrimaryOsdId;
}
