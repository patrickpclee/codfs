#include "objectmetadatamodule.hh"

#include "../config/config.hh"

#include "../storage/mongodb.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

/**
 * @brief	Default Constructor
 */
ObjectMetaDataModule::ObjectMetaDataModule(ConfigMetaDataModule* configMetaDataModule)
{
	_configMetaDataModule = configMetaDataModule;

	_collection = "Object Meta Data";

	_objectMetaDataStorage = new MongoDB();
	_objectMetaDataStorage->connect();
	_objectMetaDataStorage->setCollection(_collection);
}

/**
 * @brief	Save Object Info
 */
void ObjectMetaDataModule::saveObjectInfo(uint64_t objectId, struct ObjectMetaData objectInfo)
{
	BSONObj queryObject = BSON ("id" << (long long int)objectId);
	BSONObj insertObject = BSON ("id" << (long long int)objectId
							<< "primary" << objectInfo._primary
							<< "checksum" << objectInfo._checksum
							<< "codingScheme" << (int)objectInfo._codingScheme
							<< "codingSetting" << objectInfo._codingSetting);
	_objectMetaDataStorage->update(queryObject, insertObject);
	saveNodeList(objectId, objectInfo._nodeList);
	return ;
}

/**
 * @brief	Read Object Info
 *
 * @param	objectId	ID of the Object
 *
 * @return	Info of the Object
 */
struct ObjectMetaData ObjectMetaDataModule::readObjectInfo(uint64_t objectId)
{
	BSONObj	queryObject = BSON ("id" << (long long int)objectId);
	BSONObj result = _objectMetaDataStorage->readOne(queryObject);
	struct ObjectMetaData objectMetaData;
	objectMetaData._id = objectId;
	BSONForEach(it, result.getObjectField("nodeList")) {
		objectMetaData._nodeList.push_back((uint32_t)it.numberInt());
	}
	objectMetaData._primary = (uint32_t)result.getField("primary").numberInt();
	objectMetaData._checksum = result.getField("checksum").str();
	objectMetaData._codingScheme = (CodingScheme)result.getField("codingScheme").numberInt();
	objectMetaData._codingSetting = result.getField("codingSetting").str();

	return objectMetaData;
}


/**
 * @brief	Save Node List of a Object
 */
void ObjectMetaDataModule::saveNodeList (uint64_t objectId, vector<uint32_t> objectNodeList)
{
	vector<uint32_t>::iterator it;
	BSONObj queryObject = BSON ("id" << (long long int)objectId);
	BSONObj pushObject;
	for(it = objectNodeList.begin(); it < objectNodeList.end(); ++it) {
		//arr << *it;
		pushObject = BSON ( "$push" << BSON ("nodeList" << *it));
		//debug("Push %" PRIu64 "\n",*it);
		_objectMetaDataStorage->push(queryObject, pushObject);
	}

	return ;
}

/**
 * @brief	Read Node List of a Object
 */
vector<uint32_t> ObjectMetaDataModule::readNodeList (uint64_t objectId)
{
	vector<uint32_t> nodeList;
	BSONObj queryObject = BSON ("id" << (long long int)objectId);
	BSONObj result = _objectMetaDataStorage->readOne(queryObject);
	BSONForEach(it, result.getObjectField("nodeList")) {
		nodeList.push_back((uint32_t)it.numberInt());
	}
	return nodeList;
}

/**
 * @brief	Set Primary of a Object
 */
void ObjectMetaDataModule::setPrimary (uint64_t objectId, uint32_t primary)
{
	BSONObj queryObject = BSON ("id" << (long long int) objectId);
	BSONObj updateObject = BSON ("$set" << BSON ("primary" << primary));
	_objectMetaDataStorage->update(queryObject, updateObject);
	
	return ;
}

/**
 * @brief	Get Primary of a Object
 */
uint32_t ObjectMetaDataModule::getPrimary (uint64_t objectId)
{
	BSONObj queryObject = BSON ("id" << (long long int) objectId);
	BSONObj temp = _objectMetaDataStorage->readOne (queryObject);
	return (uint32_t)temp.getField("primary").Int();
}

/**
 * @brief	Generate a New Object ID
 */
uint64_t ObjectMetaDataModule::generateObjectId() {

	//return _configMetaDataModule->getAndInc("objectId");
	return rand();
}
