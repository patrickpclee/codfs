#include <iostream>
using namespace std;
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "repairobjectinfomsg.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

RepairObjectInfoMsg::RepairObjectInfoMsg(Communicator* communicator) :
		Message(communicator) {

}

RepairObjectInfoMsg::RepairObjectInfoMsg(Communicator* communicator,
		uint32_t sockfd, uint64_t objectId, vector<uint32_t> deadSegmentIds,
		vector<uint32_t> newOsdIds) :
		Message(communicator) {

	_sockfd = sockfd;
	_objectId = objectId;
	_deadSegmentIds = deadSegmentIds;
	_newOsdIds = newOsdIds;
}

void RepairObjectInfoMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RepairObjectInfoPro repairObjectInfoPro;

	repairObjectInfoPro.set_objectid(_objectId);

	for (uint32_t sid : _deadSegmentIds) {
		repairObjectInfoPro.add_deadsegmentids(sid);
	}

	for (uint32_t oid : _newOsdIds) {
		repairObjectInfoPro.add_newosdids(oid);
	}

	if (!repairObjectInfoPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(REPAIR_OBJECT_INFO);
	setProtocolMsg(serializedString);

}

void RepairObjectInfoMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RepairObjectInfoPro repairObjectInfoPro;
	repairObjectInfoPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = repairObjectInfoPro.objectid();

	_deadSegmentIds.clear();
	for (int i = 0; i < repairObjectInfoPro.deadsegmentids_size(); ++i) {
		_deadSegmentIds.push_back(repairObjectInfoPro.deadsegmentids(i));
	}

	_newOsdIds.clear();
	for (int i = 0; i < repairObjectInfoPro.newosdids_size(); ++i) {
		_newOsdIds.push_back(repairObjectInfoPro.newosdids(i));
	}

}

void RepairObjectInfoMsg::doHandle() {
#ifdef COMPILE_FOR_MDS

#endif
#ifdef COMPILE_FOR_OSD
	osd->repairObjectInfoProcessor(_msgHeader.requestId, _sockfd, _objectId, _deadSegmentIds, _newOsdIds);
#endif
}

void RepairObjectInfoMsg::printProtocol() {
	debug("[REPAIR_OBJECT_INFO]: %" PRIu64 "\n", _objectId);
	for (int i = 0; i < (int) _deadSegmentIds.size(); i++)
		debug("[REPAIR_OBJECT_INFO]: (%" PRIu32 ", %" PRIu32 ")\n",
				_deadSegmentIds[i], _newOsdIds[i]);
}
