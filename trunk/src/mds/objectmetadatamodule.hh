#ifndef __OBJECT_METADATA_MODULE_HH__
#define __OBJECT_METADATA_MODULE_HH__

#include <stdint.h>

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class ObjectMetaDataModule {
public:
	ObjectMetaDataModule();

	void saveNodeList (uint64_t objectId, vector<uint32_t> objectNodeList);
	void setPrimary (uint64_t objectId, uint32_t primary);
private:
	string _collection;

	MongoDB* _objectMetaDataStorage;

	//ObjectMetaDataCache *_objectMetaDataCache;
};
#endif
