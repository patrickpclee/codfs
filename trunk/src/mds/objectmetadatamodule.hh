#ifndef __OBJECT_METADATA_MODULE_HH__
#define __OBJECT_METADATA_MODULE_HH__

#include <stdint.h>
#include "../common/metadata.hh"

#include "../cache/objectmetadatacache.hh"

class ObjectMetaDataModule {
public:
	uint32_t readObjectMetaData (uint64_t objectId);
	uint32_t writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createObjectMetaData (uint64_t objectId);
	uint32_t deleteObjectMetaData (uint64_t objectId);
private:
	ObjectMetaDataCache *_objectMetaDataCache;
};
#endif
