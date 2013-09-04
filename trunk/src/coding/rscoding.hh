#ifndef __RS_CODING_HH__
#define __RS_CODING_HH__

#include "coding.hh"

class RSCoding: public Coding {
public:

	RSCoding();
	~RSCoding();

	vector<BlockData> encode(struct SegmentData segmentData, string setting);

	SegmentData decode(vector<BlockData> &blockDataList,
			block_list_t &symbolList, uint32_t segmentSize, string setting);

	block_list_t getRequiredBlockSymbols(vector<bool> blockStatus,
			uint32_t segmentSize, string setting);

	block_list_t getRepairBlockSymbols(vector<uint32_t> failedBlocks,
			vector<bool> blockStatus, uint32_t segmentSize, string setting);

	vector<BlockData> repairBlocks(vector<uint32_t> repairBlockIdList,
			vector<BlockData> &blockData, block_list_t &symbolList,
			uint32_t segmentSize, string setting);

	uint32_t getBlockCountFromSetting (string setting);

	uint32_t getParityCountFromSetting (string setting);

	uint32_t getBlockSize(uint32_t segmentSize, string setting);

	vector<BlockData> computeDelta(BlockData oldBlock, BlockData newBlock,
	        vector<offset_length_t> offsetLength, vector<uint32_t> parityVector);

	static string generateSetting(uint32_t k, uint32_t m, uint32_t w) {
		return to_string(k) + ":" + to_string(m) + ":" + to_string(w);
	}

private:
	vector<uint32_t> getParameters(string setting);
};

#endif
