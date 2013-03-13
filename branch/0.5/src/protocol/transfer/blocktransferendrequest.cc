#include "blocktransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

BlockTransferEndRequestMsg::BlockTransferEndRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

BlockTransferEndRequestMsg::BlockTransferEndRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t blockId, bool isRecovery) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_isRecovery = isRecovery;
	
}

void BlockTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::BlockTransferEndRequestPro blockTransferEndRequestPro;
	blockTransferEndRequestPro.set_segmentid(_segmentId);
	blockTransferEndRequestPro.set_blockid(_blockId);
	blockTransferEndRequestPro.set_isrecovery(_isRecovery);

	if (!blockTransferEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (BLOCK_TRANSFER_END_REQUEST);
	setProtocolMsg(serializedString);

}

void BlockTransferEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::BlockTransferEndRequestPro blockTransferEndRequestPro;
	blockTransferEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = blockTransferEndRequestPro.segmentid();
	_blockId = blockTransferEndRequestPro.blockid();
	_isRecovery = blockTransferEndRequestPro.isrecovery();

}

void BlockTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putBlockEndProcessor (_msgHeader.requestId, _sockfd, _segmentId, _blockId, _isRecovery);
#endif
}

void BlockTransferEndRequestMsg::printProtocol() {
	debug("[BLOCK_TRANSFER_END_REQUEST] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", isRecovery = %d\n", _segmentId, _blockId, _isRecovery);
}
