#include "getobjectinforequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../mds/mds.hh"

#ifdef COMPILE_FOR_MDS
extern Mds* mds;
#endif

GetObjectInfoRequestMsg::GetObjectInfoRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetObjectInfoRequestMsg::GetObjectInfoRequestMsg(Communicator* communicator,
		uint32_t dstSockfd, uint64_t objectId) :
		Message(communicator) {

	_sockfd = dstSockfd;
	_objectId = objectId;
}

void GetObjectInfoRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::GetObjectInfoRequestPro getObjectInfoRequestPro;
	getObjectInfoRequestPro.set_objectid(_objectId);

	if (!getObjectInfoRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_OBJECT_INFO_REQUEST);
	setProtocolMsg(serializedString);

}

void GetObjectInfoRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetObjectInfoRequestPro getObjectInfoRequestPro;
	getObjectInfoRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_objectId = getObjectInfoRequestPro.objectid();

}

void GetObjectInfoRequestMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
//	mds->getObjectInfoRequestProcessor (_sockfd, _objectId);
#endif
}

void GetObjectInfoRequestMsg::printProtocol() {
	debug("[GET_OBJECT_INFO_REQUEST] Object ID = %" PRIu64 "\n", _objectId);
}

vector<uint32_t> GetObjectInfoRequestMsg::getNodeList() {
	return _nodeList;
}

void GetObjectInfoRequestMsg::setNodeList(vector<uint32_t> nodeList) {
	_nodeList = nodeList;
}

CodingScheme GetObjectInfoRequestMsg::getCodingScheme() {
	return _codingScheme;
}

void GetObjectInfoRequestMsg::setCodingScheme(CodingScheme codingScheme) {
	_codingScheme = codingScheme;
}

string GetObjectInfoRequestMsg::getCodingSetting() {
	return _codingSetting;
}

void GetObjectInfoRequestMsg::setCodingSetting(string codingSetting) {
	_codingSetting = codingSetting;
}
