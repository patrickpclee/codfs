#include "segmentlocationcache.hh"

list<struct SegmentLocation> SegmentLocationCache::readSegmentLocation(
		uint64_t objectId) {
	throw new CacheMissException();
}

uint32_t SegmentLocationCache::writeSegmentLocation(uint64_t objectId,
		list<struct SegmentLocation> osdId) {
	throw new CacheMissException();
}

uint32_t SegmentLocationCache::createSegmentLocation(uint64_t objectId) {
	return 0;
}

uint32_t SegmentLocationCache::deleteSegmentLocation(uint64_t objectId) {
	return 0;
}
