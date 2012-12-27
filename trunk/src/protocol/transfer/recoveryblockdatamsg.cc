#include "recoveryblockdatamsg.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "getblockinitrequest.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

RecoveryBlockDataMsg::RecoveryBlockDataMsg(Communicator* communicator) :
		Message(communicator) {

}

RecoveryBlockDataMsg::RecoveryBlockDataMsg(Communicator* communicator,uint32_t requestId,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t blockId,
		uint32_t length) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_length = length;

}

void RecoveryBlockDataMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::RecoveryBlockDataPro recoveryRecoveryBlockDataPro;
	recoveryRecoveryBlockDataPro.set_segmentid(_segmentId);
	recoveryRecoveryBlockDataPro.set_blockid(_blockId);
	recoveryRecoveryBlockDataPro.set_length(_length);

	if (!recoveryRecoveryBlockDataPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(RECOVERY_BLOCK_DATA);
	setProtocolMsg(serializedString);

}

void RecoveryBlockDataMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::RecoveryBlockDataPro recoveryRecoveryBlockDataPro;
	recoveryRecoveryBlockDataPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = recoveryRecoveryBlockDataPro.segmentid();
	_blockId = recoveryRecoveryBlockDataPro.blockid();
	_length = recoveryRecoveryBlockDataPro.length();

}

void RecoveryBlockDataMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	BlockData blockData;
	blockData.info.segmentId = _segmentId;
	blockData.info.blockSize = _length;
	blockData.info.blockId = _blockId;
	blockData.buf = MemoryPool::getInstance().poolMalloc(_length);

	memcpy(blockData.buf, _payload, _length);
	GetBlockInitRequestMsg* getBlockInitRequestMsg =
			(GetBlockInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	getBlockInitRequestMsg->setRecoveryBlockData(blockData);
	getBlockInitRequestMsg->setStatus(READY);
#endif
}

void RecoveryBlockDataMsg::printProtocol() {
	debug(
			"[RECOVERY_BLOCK_DATA] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", length = %" PRIu32 "\n",
			_segmentId, _blockId, _length);
}
