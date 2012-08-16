#include "objectmetadatamodule.hh"

#include "../config/config.hh"

#include "../storage/mongodb.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

ObjectMetaDataModule::ObjectMetaDataModule(ConfigMetaDataModule* configMetaDataModule)
{
	_configMetaDataModule = configMetaDataModule;

	_collection = "Object Meta Data";

	_objectMetaDataStorage = new MongoDB();
	_objectMetaDataStorage->connect();
	_objectMetaDataStorage->setCollection(_collection);
}

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

void ObjectMetaDataModule::setPrimary (uint64_t objectId, uint32_t primary)
{
	BSONObj queryObject = BSON ("id" << (long long int) objectId);
	BSONObj updateObject = BSON ("$set" << BSON ("primary" << primary));
	_objectMetaDataStorage->update(queryObject, updateObject);
	
	return ;
}

uint32_t ObjectMetaDataModule::getPrimary (uint64_t objectId)
{
	BSONObj queryObject = BSON ("id" << (long long int) objectId);
	BSONObj temp = _objectMetaDataStorage->readOne (queryObject);
	return (uint32_t)temp.getField("primary").Int();
}
