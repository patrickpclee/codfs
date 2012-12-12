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
uint32_t ConfigMetaDataModule::getAndInc (const string &config)
{
	BSONObj querySegment = BSON ("id" << "config");
	BSONObj updateSegment = BSON ("$inc" << BSON (config << 1));
	BSONObj result = _configMetaDataStorage->findAndModify(querySegment, updateSegment);
	return (uint32_t)result.getField(config).numberInt();
}
