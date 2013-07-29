#ifndef __CAUCHY_CODING_HH__
#define __CAUCHY_CODING_HH__

#include "coding.hh"

class CauchyCoding: public Coding {
public:

	CauchyCoding();
	~CauchyCoding();

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

	static string generateSetting(uint32_t k, uint32_t m, uint32_t w) {
		return to_string(k) + ":" + to_string(m) + ":" + to_string(w);
	}

private:
	vector<uint32_t> getParameters(string setting);
};

#endif
