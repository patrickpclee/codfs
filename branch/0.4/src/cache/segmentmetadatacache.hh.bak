#ifndef __OBJECT_METADATA_CACHE_HH__
#define __OBJECT_METADATA_CACHE_HH__

#include "cache.hh"
#include "mm/cache_map.hpp"
#include "../common/metadata.hh"

#include <stdint.h>
#include <sstream>

class ObjectMetaDataCache : public Cache {
public:
	ObjectMetaDataCache();

	~ObjectMetaDataCache();

	ObjectMetaData* readObjectMetaData (std::string objectId);

	void writeObjectMetaData (std::string objectId, ObjectMetaData *objectMetaData);

	//uint32_t createObjectMetaData (uint64_t objectId);

	void deleteObjectMetaData (std::string objectId);

	char* read (uint64_t id);

	void write (uint64_t id, char* data);

	void deleteEntry (uint64_t id);
private:
	mm::cache_map<std::string, ObjectMetaData*> _objectMetaDataMap;
};
#endif
