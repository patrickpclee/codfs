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
		uint32_t osdSockfd, uint64_t segmentId, uint32_t blockId,
		vector<offset_length_t> symbols, bool isRecovery) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_symbols = symbols;
	_isRecovery = isRecovery;

}

void GetBlockInitRequestMsg::prepareProtocolMsg() {
	string serializedString;
	ncvfs::GetBlockInitRequestPro getBlockInitRequestPro;
	getBlockInitRequestPro.set_segmentid(_segmentId);
	getBlockInitRequestPro.set_blockid(_blockId);
	getBlockInitRequestPro.set_isrecovery(_isRecovery);

	vector<offset_length_t>::iterator it;

	for (it = _symbols.begin(); it < _symbols.end(); ++it) {
		ncvfs::OffsetLengthPro* offsetLengthPro =
				getBlockInitRequestPro.add_offsetlength();
		offsetLengthPro->set_offset((*it).first);
		offsetLengthPro->set_length((*it).second);
	}

	if (!getBlockInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(GET_BLOCK_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void GetBlockInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::GetBlockInitRequestPro getBlockInitRequestPro;
	getBlockInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = getBlockInitRequestPro.segmentid();
	_blockId = getBlockInitRequestPro.blockid();
	_isRecovery = getBlockInitRequestPro.isrecovery();

	for (int i = 0; i < getBlockInitRequestPro.offsetlength_size(); ++i) {
		offset_length_t tempOffsetLength;

		uint32_t offset = getBlockInitRequestPro.offsetlength(i).offset();
		uint32_t length = getBlockInitRequestPro.offsetlength(i).length();
		tempOffsetLength = make_pair (offset, length);

		_symbols.push_back(tempOffsetLength);
	}
}

void GetBlockInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	if (_isRecovery) {
		debug ("IS RECOVERY for segment %" PRIu64 " Block %" PRIu32 "\n", _segmentId, _blockId);
		osd->getRecoveryBlockProcessor (_msgHeader.requestId, _sockfd, _segmentId, _blockId, _symbols);
	} else {
		debug ("NOT RECOVERY for segment %" PRIu64 " Block %" PRIu32 "\n", _segmentId, _blockId);
		osd->getBlockRequestProcessor (_msgHeader.requestId, _sockfd, _segmentId, _blockId, _symbols);
	}
#endif
}

void GetBlockInitRequestMsg::printProtocol() {
	debug(
			"[GET_BLOCK_INIT] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 "\n",
			_segmentId, _blockId);
}

void GetBlockInitRequestMsg::setRecoveryBlockData (BlockData blockData) {
	_recoveryBlockData = blockData;
}

BlockData GetBlockInitRequestMsg::getRecoveryBlockData () {
	return _recoveryBlockData;
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
