#ifndef __DUMMYENCODE_HH__
#define __DUMMYENCODE_HH__

#include "encodingbehaviour.hh"

using namespace std;

class DummyEncode : public EncodingBehaviour {
public:
	DummyEncode();
	~DummyEncode();
	vector<struct SegmentData> encode(struct ObjectData objectData);
};
#endif
