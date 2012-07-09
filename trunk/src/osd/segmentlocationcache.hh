#ifndef __SEGMENT_LOCATION_CACHE_HH__
#define __SEGMENT_LOCATION_CACHE_HH__

#include <list>
#include "../cache/cache.hh"

#include <stdint.h>

class SegmentLocationCache : public Cache {
public:
	list <uint32_t> readSegmentLocation (uint64_t objectId);
	uint32_t writeSegmentLocation (uint64_t objectId, list<uint32_t> osdId);
	uint32_t createSegmentLocation (uint64_t objectId);
	uint32_t deleteSegmentLocation (uint64_t objectId);
private:
};
#endif
