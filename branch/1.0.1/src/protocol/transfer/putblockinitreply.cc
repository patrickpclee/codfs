#include "putblockinitreply.hh"
#include "putblockinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

PutBlockInitReplyMsg::PutBlockInitReplyMsg(Communicator* communicator) :
		Message(communicator) {

}

PutBlockInitReplyMsg::PutBlockInitReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t osdSockfd, uint64_t segmentId, uint32_t blockId) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	
}

void PutBlockInitReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutBlockInitReplyPro putBlockInitReplyPro;
	putBlockInitReplyPro.set_segmentid(_segmentId);

	if (!putBlockInitReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (PUT_BLOCK_INIT_REPLY);
	setProtocolMsg(serializedString);

}

void PutBlockInitReplyMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutBlockInitReplyPro putBlockInitReplyPro;
	putBlockInitReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = putBlockInitReplyPro.segmentid();
	_blockId = putBlockInitReplyPro.blockid();

}

void PutBlockInitReplyMsg::doHandle() {

	PutBlockInitRequestMsg* putBlockInitRequestMsg =
			(PutBlockInitRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	putBlockInitRequestMsg->setStatus(READY);
}

void PutBlockInitReplyMsg::printProtocol() {
	debug("[PUT_BLOCK_INIT_REPLY] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 "\n", _segmentId, _blockId);
}
