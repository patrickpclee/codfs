#ifndef __OBJECT_METADATA_MODULE_HH__
#define __OBJECT_METADATA_MODULE_HH__

#include <stdint.h>
#include "../common/metadata.hh"

#include "../cache/objectmetadatacache.hh"

class ObjectMetaDataModule {
public:
	void readObjectMetaData (uint64_t objectId);
	void writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	void createObjectMetaData (uint64_t objectId);
	void deleteObjectMetaData (uint64_t objectId);
private:
	ObjectMetaDataCache *_objectMetaDataCache;
};
#endif
