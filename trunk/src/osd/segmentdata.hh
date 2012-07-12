#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <string>

using namespace std;

struct SegmentData {
	uint64_t objectId;
	uint32_t segmentId;
	uint32_t offsetInObject;
	uint32_t length;
	string segmentPath;
	vector <unsigned char>* buf;
};

#endif
