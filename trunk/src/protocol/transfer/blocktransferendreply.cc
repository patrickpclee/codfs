#include "blocktransferendreply.hh"
#include "blocktransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

BlockTransferEndReplyMsg::BlockTransferEndReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

BlockTransferEndReplyMsg::BlockTransferEndReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t dstSockfd, uint64_t segmentId, uint32_t blockId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = dstSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	
}

void BlockTransferEndReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::BlockTransferEndReplyPro blockTransferEndReplyPro;
	blockTransferEndReplyPro.set_segmentid(_segmentId);
	blockTransferEndReplyPro.set_blockid(_blockId);

	if (!blockTransferEndReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (BLOCK_TRANSFER_END_REPLY);
	setProtocolMsg(serializedString);

}

void BlockTransferEndReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::BlockTransferEndReplyPro blockTransferEndReplyPro;
	blockTransferEndReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = blockTransferEndReplyPro.segmentid();
	_blockId = blockTransferEndReplyPro.blockid();

}

void BlockTransferEndReplyMsg::doHandle() {
		BlockTransferEndRequestMsg* putBlockEndRequestMsg =
			(BlockTransferEndRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putBlockEndRequestMsg->setStatus(READY);
}

void BlockTransferEndReplyMsg::printProtocol() {
	debug("[BLOCK_TRANSFER_END_REPLY] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 "\n", _segmentId, _blockId);
}
