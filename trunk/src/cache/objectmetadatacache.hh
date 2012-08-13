#ifndef __OBJECT_METADATA_CACHE_HH__
#define __OBJECT_METADATA_CACHE_HH__

#include "cache.hh"
#include "mm/cache_map.hpp"
#include "../common/metadata.hh"

#include <stdint.h>

class ObjectMetaDataCache : public Cache {
public:
	ObjectMetaDataCache();

	~ObjectMetaDataCache();

	ObjectMetaData* readObjectMetaData (uint64_t objectId);

	void writeObjectMetaData (uint64_t objectId, ObjectMetaData *objectMetaData);

	//uint32_t createObjectMetaData (uint64_t objectId);

	void deleteObjectMetaData (uint64_t objectId);

	char* read (uint64_t id);

	void write (uint64_t id, char* data);

	void deleteEntry (uint64_t id);
private:
	mm::cache_map<uint64_t, ObjectMetaData*> _objectMetaDataMap;
};
#endif
