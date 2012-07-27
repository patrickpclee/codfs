#ifndef __DUMMYDECODE_HH__
#define __DUMMYDECODE_HH__

#include "decodingbehaviour.hh"

using namespace std;

class DummyDecode : public DecodingBehaviour {
public:
	DummyDecode();
	~DummyDecode();
	struct ObjectData decode(vector<struct SegmentData> segmentData);
};
#endif
