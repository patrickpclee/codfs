#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <stdint.h>
#include <string>

using namespace std;

struct SegmentInfo {
	uint64_t segmentId;
	uint32_t segmentSize;
	string segmentPath;
};

struct SegmentData {
	struct SegmentInfo info;
	char* buf;
};

#endif
