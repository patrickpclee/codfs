#ifndef __RAID0CODING_HH__
#define __RAID0CODING_HH__

#include "coding.hh"

class Raid0Coding: public Coding {
public:

	Raid0Coding();
	~Raid0Coding();

	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);

	struct ObjectData decode(vector<struct SegmentData> segmentData,
			string setting);

	vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus);

	static string generateSetting(int noOfStrips) {
		return to_string(noOfStrips);
	}

private:
	uint32_t getNoOfStrips(string setting);
};

#endif
