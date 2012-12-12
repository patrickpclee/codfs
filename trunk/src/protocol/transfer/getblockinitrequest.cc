#include "getblockinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

GetBlockInitRequestMsg::GetBlockInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

GetBlockInitRequestMsg::GetBlockInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t blockId) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	
}

void GetBlockInitRequestMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::GetBlockInitRequestPro getBlockInitRequestPro;
	getBlockInitRequestPro.set_segmentid(_segmentId);
	getBlockInitRequestPro.set_blockid(_blockId);

	if (!getBlockInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (GET_BLOCK_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetBlockInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetBlockInitRequestPro getBlockInitRequestPro;
	getBlockInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getBlockInitRequestPro.segmentid();
	_blockId = getBlockInitRequestPro.blockid();
}

void GetBlockInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->getBlockRequestProcessor (_msgHeader.requestId, _sockfd, _segmentId, _blockId);
#endif
}

void GetBlockInitRequestMsg::printProtocol() {
	debug(
			"[GET_BLOCK_INIT] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 "\n",
			_segmentId, _blockId);
}

/*
void GetBlockInitRequestMsg::setBlockSize(uint32_t blockSize) {
	_blockSize = blockSize;
}

uint32_t GetBlockInitRequestMsg::getBlockSize() {
	return _blockSize;
}

void GetBlockInitRequestMsg::setChunkCount(uint32_t chunkCount) {
	_chunkCount = chunkCount;
}

uint32_t GetBlockInitRequestMsg::getChunkCount() {
	return _chunkCount;
}
*/
