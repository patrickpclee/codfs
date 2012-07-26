#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <string>

using namespace std;

struct SegmentInfo {
	uint64_t objectId;
	uint32_t segmentId;
	uint32_t segmentSize;
	string segmentPath;
	uint32_t offsetInObject;
};

struct SegmentData {
	struct SegmentInfo info;
	char* buf;
};

#endif
