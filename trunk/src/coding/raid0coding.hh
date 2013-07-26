#ifndef __RAID0CODING_HH__
#define __RAID0CODING_HH__

#include "coding.hh"

class Raid0Coding: public Coding {
public:

	Raid0Coding();
	~Raid0Coding();

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

	static string generateSetting(int raid0_n) {
		return to_string(raid0_n);
	}

private:
	uint32_t getParameters(string setting);
};

#endif
