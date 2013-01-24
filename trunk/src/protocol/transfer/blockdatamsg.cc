#include "blockdatamsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
#include "../client/client.hh"
extern Client* client;
#endif

BlockDataMsg::BlockDataMsg(Communicator* communicator) :
		Message(communicator) {

}

BlockDataMsg::BlockDataMsg(Communicator* communicator, uint32_t osdSockfd,
		uint64_t segmentId, uint32_t blockId, uint64_t offset, uint32_t length,
		bool isRecovery) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_offset = offset;
	_length = length;
	_isRecovery = isRecovery;

}

void BlockDataMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::BlockDataPro blockDataPro;
	blockDataPro.set_segmentid(_segmentId);
	blockDataPro.set_blockid(_blockId);
	blockDataPro.set_offset(_offset);
	blockDataPro.set_length(_length);
	blockDataPro.set_isrecovery(_isRecovery);

	if (!blockDataPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(BLOCK_DATA);
	setProtocolMsg(serializedString);

}

void BlockDataMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::BlockDataPro blockDataPro;
	blockDataPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = blockDataPro.segmentid();
	_blockId = blockDataPro.blockid();
	_offset = blockDataPro.offset();
	_length = blockDataPro.length();
	_isRecovery = blockDataPro.isrecovery();

}

void BlockDataMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putBlockDataProcessor(_msgHeader.requestId, _sockfd, _segmentId, _blockId, _offset, _length, _payload);
#endif
}

void BlockDataMsg::printProtocol() {
	debug(
			"[BLOCK_DATA] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", offset = %" PRIu64 ", length = %" PRIu32 ", isRecovery = %d\n",
			_segmentId, _blockId, _offset, _length, _isRecovery);
}
