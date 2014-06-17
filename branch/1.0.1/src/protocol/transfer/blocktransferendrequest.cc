#include "blocktransferendrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

BlockTransferEndRequestMsg::BlockTransferEndRequestMsg(
		Communicator* communicator) :
		Message(communicator) {

}

BlockTransferEndRequestMsg::BlockTransferEndRequestMsg(
        Communicator* communicator, uint32_t osdSockfd, uint64_t segmentId,
        uint32_t blockId, DataMsgType dataMsgType, string updateKey,
        vector<offset_length_t> offsetLength, vector<BlockLocation> parityList,
        CodingScheme codingScheme, string codingSetting, uint64_t segmentSize) :
        Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_blockId = blockId;
	_dataMsgType = (DataMsgType) dataMsgType;
	_updateKey = updateKey;
	_offsetLength = offsetLength;
	_parityList = parityList;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
	_segmentSize = segmentSize;
}

void BlockTransferEndRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::BlockTransferEndRequestPro blockTransferEndRequestPro;
	blockTransferEndRequestPro.set_segmentid(_segmentId);
	blockTransferEndRequestPro.set_blockid(_blockId);
	blockTransferEndRequestPro.set_datamsgtype(
			(ncvfs::DataMsgPro_DataMsgType) _dataMsgType);
	blockTransferEndRequestPro.set_updatekey(_updateKey);
    blockTransferEndRequestPro.set_codingscheme(
            (ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
    blockTransferEndRequestPro.set_codingsetting(_codingSetting);
    blockTransferEndRequestPro.set_segmentsize(_segmentSize);

	vector<offset_length_t>::iterator it;
	for (it = _offsetLength.begin(); it < _offsetLength.end(); ++it) {
		ncvfs::OffsetLengthPro* offsetLengthPro =
				blockTransferEndRequestPro.add_offsetlength();
		offsetLengthPro->set_offset((*it).first);
		offsetLengthPro->set_length((*it).second);
	}

    vector<BlockLocation>::iterator it1;
	for (it1 = _parityList.begin(); it1 < _parityList.end(); ++it1) {
		ncvfs::BlockLocationPro* blockLocationPro =
				blockTransferEndRequestPro.add_blocklocation();
        blockLocationPro->set_osdid((*it1).osdId);
        blockLocationPro->set_blockid((*it1).blockId);
	}

	if (!blockTransferEndRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(BLOCK_TRANSFER_END_REQUEST);
	setProtocolMsg(serializedString);

}

void BlockTransferEndRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::BlockTransferEndRequestPro blockTransferEndRequestPro;
	blockTransferEndRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = blockTransferEndRequestPro.segmentid();
	_blockId = blockTransferEndRequestPro.blockid();
	_dataMsgType = (DataMsgType) blockTransferEndRequestPro.datamsgtype();
	_updateKey = blockTransferEndRequestPro.updatekey();
	_codingScheme = (CodingScheme) blockTransferEndRequestPro.codingscheme();
	_codingSetting = blockTransferEndRequestPro.codingsetting();
	_segmentSize = blockTransferEndRequestPro.segmentsize();

	for (int i = 0; i < blockTransferEndRequestPro.offsetlength_size(); ++i) {
		offset_length_t tempOffsetLength;

		uint32_t offset = blockTransferEndRequestPro.offsetlength(i).offset();
		uint32_t length = blockTransferEndRequestPro.offsetlength(i).length();
		tempOffsetLength = make_pair(offset, length);

		_offsetLength.push_back(tempOffsetLength);
	}

	for (int i = 0; i < blockTransferEndRequestPro.blocklocation_size(); ++i) {
	        struct BlockLocation tempBlockLocation;

	        tempBlockLocation.osdId =
	                blockTransferEndRequestPro.blocklocation(i).osdid();
	        tempBlockLocation.blockId =
	                blockTransferEndRequestPro.blocklocation(i).blockid();

	        _parityList.push_back(tempBlockLocation);
	    }

}

void BlockTransferEndRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putBlockEndProcessor(_msgHeader.requestId, _sockfd, _segmentId,
			_blockId, _dataMsgType, _updateKey, _offsetLength, _parityList, _codingScheme, _codingSetting, _segmentSize);
#endif
}

void BlockTransferEndRequestMsg::printProtocol() {
	debug(
            "[BLOCK_TRANSFER_END_REQUEST] Segment ID = %" PRIu64 ", Block ID = %" PRIu32 ", dataMsgType = %d, codingScheme = %d, codingSetting = %s\n",
            _segmentId, _blockId, _dataMsgType, _codingScheme,
            _codingSetting.c_str());
}
