#ifndef __CONFIG_METADATA_MODULE_HH__
#define __CONFIG_METADATA_MODULE_HH__

#include <stdint.h>

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class ConfigMetaDataModule {
public:
	/**
	 * @brief	Default Constructor
	 */
	ConfigMetaDataModule();

	/**
	 * @brief	Get a Setting and Increment
	 *
	 * @param	config	Filed Name of Config
	 */
	uint32_t getAndInc (const string &config);
private:

	/// Collection
	string _collection;

	/// Underlying Meta Data Storage
	MongoDB* _configMetaDataStorage;
};

#endif
