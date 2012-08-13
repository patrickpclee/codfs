#ifndef __RAID1ENCODE_HH__
#define __RAID1ENCODE_HH__

#include <stdint.h>
#include "encodingbehaviour.hh"

using namespace std;

class Raid1Encode : public EncodingBehaviour {
public:
	Raid1Encode(uint32_t noOfReplications);
	~Raid1Encode();
	vector<struct SegmentData> encode(struct ObjectData objectData);
private:
	uint32_t _noOfReplications;
};
#endif
