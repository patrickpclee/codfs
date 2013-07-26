#ifndef __EMBR_CODING_HH__
#define __EMBR_CODING_HH__

#include "coding.hh"
#include "rscoding.hh"

class EMBRCoding: public RSCoding {
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

	uint32_t getBlockSize(uint32_t segmentSize, string setting);

	static string generateSetting(uint32_t n, uint32_t k, uint32_t w) {
		const uint32_t rs_k = k * (n - 1) - k * (k - 1) / 2;
		const uint32_t rs_m = n * (n - 1) / 2 - k * (2 * n - k - 1) / 2;
		return to_string(n) + ":" + to_string(k) + ":" + to_string(w) + ":" + to_string(rs_k) + ":" + to_string(rs_m);
	}

private:
	vector<uint32_t> getParameters(string setting);

	inline void convertToRS(vector<BlockData> &blockDataList, block_list_t &symbolList, uint32_t segmentSize, string setting, vector<BlockData> &RSBlockDataList, block_list_t &RSSymbolList, string &RSSetting);
};

#endif
