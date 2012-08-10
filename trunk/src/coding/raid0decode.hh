#ifndef __RAID0DECODE_HH__
#define __RAID0DECODE_HH__

#include <stdint.h>
#include "decodingbehaviour.hh"

using namespace std;

class Raid0Decode : public DecodingBehaviour {
public:
	Raid0Decode(uint32_t noOfStrips);
	~Raid0Decode();
	struct ObjectData decode(vector<struct SegmentData> segmentData);
private:
	uint32_t _noOfStrips;
};
#endif
