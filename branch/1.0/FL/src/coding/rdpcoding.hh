#ifndef __RDP_CODING_HH__
#define __RDP_CODING_HH__

#include "coding.hh"
#include "evenoddcoding.hh"

class RDPCoding: public EvenOddCoding {
public:

	RDPCoding();
	~RDPCoding();

	vector<BlockData> encode(struct SegmentData segmentData, string setting);

	block_list_t getRepairBlockSymbols(vector<uint32_t> failedBlocks,
			vector<bool> blockStatus, uint32_t segmentSize, string setting);

	vector<BlockData> repairBlocks(vector<uint32_t> repairBlockIdList,
			vector<BlockData> &blockData, block_list_t &symbolList,
			uint32_t segmentSize, string setting);

	virtual uint32_t getBlockSize(uint32_t segmentSize, string setting);

    uint32_t getParityCountFromSetting (string setting);

    vector<BlockData> computeDelta(BlockData oldBlock, BlockData newBlock,
        vector<offset_length_t> offsetLength, vector<uint32_t> parityVector);

protected:
	virtual uint32_t getSymbolSize(uint32_t blockSize, uint32_t k) {
		return blockSize / k;
	}

	virtual char** repairDataBlocks(vector<BlockData> &blockDataList,
			block_list_t &symbolList, uint32_t segmentSize, string setting, bool recovery = false);

    vector <uint32_t> deltaMap[4][4];
};

#endif
