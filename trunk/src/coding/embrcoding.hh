#ifndef __EMBR_CODING_HH__
#define __EMBR_CODING_HH__

#include "coding.hh"

class EMBRCoding: public Coding {
public:

	EMBRCoding();
	~EMBRCoding();

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

	static string generateSetting(uint32_t n, uint32_t k, uint32_t w) {
		return to_string(n) + ":" + to_string(k) + ":" + to_string(w);
	}

private:
	vector<uint32_t> getParameters(string setting);
};

#endif
