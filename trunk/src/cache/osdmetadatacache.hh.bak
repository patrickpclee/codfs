#ifndef __OSD_METADATA_CACHE_HH__
#define __OSD_METADATA_CACHE_HH__

#include "cache.hh"
#include "mm/cache_map.hpp"
#include "../common/metadata.hh"

#include <stdint.h>

class OsdMetaDataCache : public Cache {
public:
	OsdMetaDataCache();

	~OsdMetaDataCache();

	SegmentMetaData* readOsdMetaData (uint32_t osdId);

	void writeOsdMetaData (uint32_t osdId, SegmentMetaData *osdMetaData);

	//uint32_t createObjectMetaData (uint64_t objectId);

	void deleteOsdMetaData (uint32_t osdId);

	char* read (uint64_t id);

	void write (uint64_t id, char* data);

	void deleteEntry (uint64_t id);
private:
	mm::cache_map<uint32_t, SegmentMetaData*> _osdMetaDataMap;
};
#endif
