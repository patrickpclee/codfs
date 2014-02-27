#ifndef __EVENODD_CODING_HH__
#define __EVENODD_CODING_HH__

#include "coding.hh"

class EvenOddCoding: public Coding {
public:

	EvenOddCoding();
	~EvenOddCoding();

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

	virtual uint32_t getBlockSize(uint32_t segmentSize, string setting);

	static string generateSetting(uint32_t n) {
		return to_string(n);
	}


protected:
	virtual uint32_t getSymbolSize(uint32_t blockSize, uint32_t k) {
		return blockSize / (k - 1);
	}

	uint32_t getParameters(string setting);
	virtual char** repairDataBlocks(vector<BlockData> &blockDataList,
			block_list_t &symbolList, uint32_t segmentSize, string setting, bool recovery = false);
};

#endif
