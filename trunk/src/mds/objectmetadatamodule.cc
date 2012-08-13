#include "objectmetadatamodule.hh"

#include "../config/config.hh"

#include "../storage/mongodb.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

ObjectMetaDataModule::ObjectMetaDataModule()
{
	_collection = "Object Meta Data";

	_objectMetaDataStorage = new MongoDB();
	_objectMetaDataStorage->connect();
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
		_objectMetaDataStorage->push(_collection, queryObject, pushObject);
	}

	return ;
}

void ObjectMetaDataModule::setPrimary (uint64_t objectId, uint32_t primary)
{
	BSONObj queryObject = BSON ("id" << (long long int) objectId);
	BSONObj updateObject = BSON ("$set" << BSON ("primary" << primary));
	_objectMetaDataStorage->update(_collection, queryObject, updateObject);
	
	return ;
}
