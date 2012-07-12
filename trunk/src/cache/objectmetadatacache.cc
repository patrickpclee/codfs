#include "objectmetadatacache.hh"

uint32_t ObjectMetaDataCache::readObjectMetaData (uint64_t objectId){
	throw new CacheMissException();
}

uint32_t ObjectMetaDataCache::writeObjectMetaData (uint64_t objectId, ObjectMetaData objectMetaData){
	throw new CacheMissException();
}

uint32_t ObjectMetaDataCache::createObjectMetaData (uint64_t objectId){
	return 0;
}

uint32_t ObjectMetaDataCache::deleteObjectMetaData (uint64_t objectId){
	return 0;
}
