#ifndef __OBJECT_METADATA_CACHE_HH__
#define __OBJECT_METADATA_CACHE_HH__

#include "../cache/cache.hh"
#include "../common/metadata.hh"

#include <stdint.h>

class ObjectMetaDataCache : public Cache {
public:
	uint32_t readObjectMetaData (uint64_t objectId);
	uint32_t writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createObjectMetaData (uint64_t objectId);
	uint32_t deleteObjectMetaData (uint64_t objectId);
private:
};
#endif
