#ifndef __SEGMENT_LOCATION_CACHE_HH__
#define __SEGMENT_LOCATION_CACHE_HH__

#include <list>
#include "../cache/cache.hh"

#include <stdint.h>

struct SegmentLocation {
	uint32_t osdId;
	uint32_t segmentId;
};

class SegmentLocationCache: public Cache {
public:
	list<struct SegmentLocation> readSegmentLocation(uint64_t objectId);
	uint32_t writeSegmentLocation(uint64_t objectId,
			list<struct SegmentLocation> osdId);
	uint32_t createSegmentLocation(uint64_t objectId);
	uint32_t deleteSegmentLocation(uint64_t objectId);
private:
};
#endif
