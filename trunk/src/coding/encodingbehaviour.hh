#ifndef __CODINGBEHAVIOUR_HH__
#define __CODINGBEHAVIOUR_HH__

#include <vector>

using namespace std;

class EncodingBehaviour {
public:
	EncodingBehaviour();
	virtual ~EncodingBehaviour();
	virtual vector<struct SegmentData> encode(struct ObjectData objectData) = 0;
};

#endif
