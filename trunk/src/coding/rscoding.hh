#ifndef __RS_CODING_HH__
#define __RS_CODING_HH__

#include "coding.hh"

class RSCoding: public Coding {
public:

	RSCoding();
	~RSCoding();

	vector<struct BlockData> encode(struct SegmentData segmentData,
			string setting);

	struct SegmentData decode(vector<struct BlockData> &blockData,
			vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
			string setting);

	vector<uint32_t> getRequiredBlockIds(string setting,
			vector<bool> secondaryOsdStatus);

	vector<uint32_t> getRepairSrcBlockIds(string setting,
			vector<uint32_t> failedBlocks, vector<bool> blockStatus);

	vector<struct BlockData> repairBlocks(
			vector<uint32_t> failedBlocks,
			vector<struct BlockData> &repairSrcBlocks,
			vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
			string setting);

	static string generateSetting(uint32_t k, uint32_t m, uint32_t w) {
		return to_string(k)+":"+to_string(m)+":"+to_string(w);
	}

private:
	vector<uint32_t> getParams(string setting);
};

#endif
