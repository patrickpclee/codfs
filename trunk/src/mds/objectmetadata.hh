#ifndef __CACHE_OBJECT_METADATA_HH__
#define __CACHE_OBJECT_METADATA_HH__

#include <stdint.h>

class ObjectMetaData {
public:
	uint32_t readObjectMetaData (uint64_t objectId);
//	writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createObjectMetaData (uint64_t objectId);
	uint32_t deleteObjectMetaData (uint64_t objectId);
private:
};
#endif