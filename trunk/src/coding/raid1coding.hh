#ifndef __RAID1CODING_HH__
#define __RAID1CODING_HH__

#include "coding.hh"

class Raid1Coding: public Coding {
public:
	Raid1Coding();
	~Raid1Coding();

	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);

	struct ObjectData decode(vector<struct SegmentData> &segmentData,
			vector<uint32_t> &requiredSegments, uint32_t objectSize,
			string setting);

	vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus);

	vector<uint32_t> getRepairSrcSegmentIds(string setting,
			vector<uint32_t> failedSegments, vector<bool> segmentStatus);

	vector<struct SegmentData> repairSegments(
			vector<struct SegmentData> &repairSrcSegments,
			vector<uint32_t> &repairSrcSegmentId, uint32_t objectSize,
			string setting);

	static string generateSetting(int raid1_n) {
		return to_string(raid1_n);
	}

private:
	uint32_t getParameters(string setting);
};

#endif
