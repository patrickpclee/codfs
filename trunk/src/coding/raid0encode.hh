#ifndef __RAID0ENCODE_HH__
#define __RAID0ENCODE_HH__

#include <stdint.h>
#include "encodingbehaviour.hh"

using namespace std;

class Raid0Encode : public EncodingBehaviour {
public:
	Raid0Encode(uint32_t noOfStrips);
	~Raid0Encode();
	vector<struct SegmentData> encode(struct ObjectData objectData);
private:
	uint32_t _noOfStrips;
};
#endif
