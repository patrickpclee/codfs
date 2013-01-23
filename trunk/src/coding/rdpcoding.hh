#ifndef __RDP_CODING_HH__
#define __RDP_CODING_HH__

#include "coding.hh"

class RDPCoding: public Coding {
public:

	RDPCoding();
	~RDPCoding();

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

	static string generateSetting(uint32_t n) {
		return to_string(n);
	}

private:
	uint32_t getParameters(string setting);
	char** repairDataBlocks(vector<BlockData> &blockDataList,
			block_list_t &symbolList, uint32_t segmentSize, string setting);
};

#endif
