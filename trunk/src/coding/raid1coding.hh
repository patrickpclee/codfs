#ifndef __RAID1CODING_HH__
#define __RAID1CODING_HH__

#include "coding.hh"

class Raid1Coding: public Coding {
public:
	Raid1Coding();
	~Raid1Coding();
	void display();
	struct ObjectData decode(vector<struct SegmentData> segmentData,
			string setting);
	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);
	uint32_t getNoOfReplications(string setting);

	static string generateSetting(int noOfReplications) {
		return to_string(noOfReplications);
	}

private:
	uint32_t _noOfReplications;
};

#endif
