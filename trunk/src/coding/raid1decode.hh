#ifndef __RAID1DECODE_HH__
#define __RAID1DECODE_HH__

#include <stdint.h>
#include "decodingbehaviour.hh"

using namespace std;

class Raid1Decode : public DecodingBehaviour {
public:
	Raid1Decode(uint32_t noOfReplications);
	~Raid1Decode();
	struct ObjectData decode(vector<struct SegmentData> segmentData);
private:
	uint32_t _noOfReplications;
};
#endif
