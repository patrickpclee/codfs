#ifndef __RAID1CODING_HH__
#define __RAID1CODING_HH__

#include "coding.hh"

class Raid1Coding: public Coding {
public:
	Raid1Coding();
	~Raid1Coding();

	struct ObjectData decode(vector<struct SegmentData> segmentData,
			vector<uint32_t> requiredSegments, string setting);

	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);

	vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus);

	static string generateSetting(int noOfReplications) {
		return to_string(noOfReplications);
	}

private:
	uint32_t getNoOfReplications(string setting);
};

#endif
