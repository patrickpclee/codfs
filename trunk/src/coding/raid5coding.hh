#ifndef __RAID5CODING_HH__
#define __RAID5CODING_HH__

#include "coding.hh"

class Raid5Coding: public Coding {
public:

	Raid5Coding();
	~Raid5Coding();

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

	static string generateSetting(int raid5_n) {
		return to_string(raid5_n);
	}

private:
	uint32_t getParameters(string setting);
};

#endif
