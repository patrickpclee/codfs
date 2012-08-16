#ifndef __CONFIG_METADATA_MODULE_HH__
#define __CONFIG_METADATA_MODULE_HH__

#include <stdint.h>

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class ConfigMetaDataModule {
public:
	ConfigMetaDataModule();

	uint32_t getAndInc (string config);
private:
	string _collection;

	MongoDB* _configMetaDataStorage;
};

#endif
