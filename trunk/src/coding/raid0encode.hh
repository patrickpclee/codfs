#ifndef __RAID0ENCODE_HH__
#define __RAID0ENCODE_HH__

#include "encodingbehaviour.hh"

using namespace std;

class Raid0Encode : public EncodingBehaviour {
public:
	Raid0Encode();
	~Raid0Encode();
	vector<struct SegmentData> encode(struct ObjectData objectData);
};
#endif
