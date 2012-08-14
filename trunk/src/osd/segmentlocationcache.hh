#ifndef __SEGMENT_LOCATION_CACHE_HH__
#define __SEGMENT_LOCATION_CACHE_HH__

#include <vector>
#include "../cache/cache.hh"

#include <stdint.h>

struct SegmentLocation {
	uint32_t osdId;
	uint32_t segmentId;
};

class SegmentLocationCache: public Cache {
public:
	char * read(uint64_t id);
	void write(uint64_t id, char* data);
	void deleteEntry(uint64_t id);
private:
};
#endif
