#ifndef __RAID0CODING_HH__
#define __RAID0CODING_HH__

#include "coding.hh"

class Raid0Coding: public Coding {
public:

	Raid0Coding();
	~Raid0Coding();

	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);

	struct ObjectData decode(vector<struct SegmentData> &segmentData,
			vector<uint32_t> &requiredSegments, uint32_t objectSize,
			string setting);

	vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus);

	static string generateSetting(int raid0_n) {
		return to_string(raid0_n);
	}

private:
	uint32_t getParameters(string setting);
};

#endif
