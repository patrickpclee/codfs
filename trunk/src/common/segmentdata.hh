#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <string>
#include <stdint.h>

using namespace std;

struct SegmentInfo {
	uint64_t objectId;
	uint32_t segmentId;
	uint32_t segmentSize;
};

struct SegmentData {
	struct SegmentInfo info;
	char* buf;
};

#endif
