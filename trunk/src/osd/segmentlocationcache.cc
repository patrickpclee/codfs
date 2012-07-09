#include "segmentlocationcache.hh"

list <uint32_t> SegmentLocationCache::readSegmentLocation (uint64_t objectId){
	throw new CacheMissException();
}

uint32_t SegmentLocationCache::writeSegmentLocation(uint64_t objectId, list<uint32_t> osdId) {
	throw new CacheMissException();
}

uint32_t SegmentLocationCache::createSegmentLocation(uint64_t objectId) {
	return 0;
}

uint32_t SegmentLocationCache::deleteSegmentLocation(uint64_t objectId) {
	return 0;
}
