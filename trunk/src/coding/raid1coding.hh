#ifndef __RAID1CODING_HH__
#define __RAID1CODING_HH__

#include "coding.hh"

class Raid1Coding: public Coding {
public:
	Raid1Coding();
	~Raid1Coding();

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

	static string generateSetting(int raid1_n) {
		return to_string(raid1_n);
	}

private:
	uint32_t getParameters(string setting);
};

#endif
