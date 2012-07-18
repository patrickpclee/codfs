#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <string>

using namespace std;

struct SegmentInfo {
	uint64_t objectId;
	uint32_t segmentId;
	uint32_t segmentLength;
	string segmentPath;
};

struct SegmentData {
	struct SegmentInfo info;
	uint32_t offsetInObject;
	char* buf;
};

#endif
