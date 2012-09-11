#include "configmetadatamodule.hh"

using namespace mongo;

/**
 * @brief	Default Constructor
 */
ConfigMetaDataModule::ConfigMetaDataModule(){
	_collection = "Configuration";

	_configMetaDataStorage = new MongoDB();
	_configMetaDataStorage->connect();
	_configMetaDataStorage->setCollection(_collection);
}

/**
 * @brief	Get a Setting and Increment
 */
uint32_t ConfigMetaDataModule::getAndInc (string config)
{
	BSONObj queryObject = BSON ("id" << "config");
	BSONObj updateObject = BSON ("$inc" << BSON (config << 1));
	BSONObj result = _configMetaDataStorage->findAndModify(queryObject, updateObject);
	return (uint32_t)result.getField(config).numberInt();
}
