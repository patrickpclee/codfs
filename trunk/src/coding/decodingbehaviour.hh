#ifndef __DECODINGBEHAVIOUR_HH__
#define __DECODINGBEHAVIOUR_HH__

#include <vector>

using namespace std;

class DecodingBehaviour {
public:
	DecodingBehaviour();
	virtual ~DecodingBehaviour();
	virtual struct ObjectData decode(vector<struct SegmentData> segmentDataList) = 0;
};

#endif
