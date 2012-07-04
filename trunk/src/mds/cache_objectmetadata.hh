#ifndef __CACHE_OBJECT_METADATA_HH__
#define __CACHE_OBJECT_METADATA_HH__

#include "../cache/cache.hh"

#include <stdint.h>

class ObjectMetaDataCache : public Cache {
public:
	uint32_t readObjectMetaData (uint64_t objectId);
//	writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createObjectMetaData (uint64_t objectId);
	uint32_t deleteObjectMetaData (uint64_t objectId);
private:
};
#endif
