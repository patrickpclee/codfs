#ifndef __RAID0DECODE_HH__
#define __RAID0DECODE_HH__

#include "decodingbehaviour.hh"

using namespace std;

class Raid0Decode : public DecodingBehaviour {
public:
	Raid0Decode();
	~Raid0Decode();
	struct ObjectData decode(vector<struct SegmentData> segmentData);
};
#endif
