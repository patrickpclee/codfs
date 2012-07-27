#ifndef __DUMMYENCODE_HH__
#define __DUMMYENCODE_HH__

#include "codingbehaviour.hh"

using namespace std;

class DummyEncode : public CodingBehaviour {
public:
	vector<struct SegmentData> encode(struct ObjectData objectData);
};
#endif
