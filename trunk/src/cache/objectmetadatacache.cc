#include <thread>
#include "objectmetadatacache.hh"

std::mutex objectCacheMutex;

ObjectMetaDataCache::ObjectMetaDataCache(){
	_objectMetaDataMap.clear();
}

ObjectMetaDataCache::~ObjectMetaDataCache(){
	_objectMetaDataMap.clear();
}

ObjectMetaData* ObjectMetaDataCache::readObjectMetaData(uint64_t objectId){
	mm::cache_map<uint64_t, ObjectMetaData*>::iterator _it = _objectMetaDataMap.find(objectId);
	if (_it != _objectMetaDataMap.end()) {
		return _it->second;
	}
	return NULL;
	//TODO cache missing
}

void ObjectMetaDataCache::writeObjectMetaData (uint64_t objectId, ObjectMetaData *objectMetaData){
	_objectMetaDataMap.insert(objectId,objectMetaData);
}

void ObjectMetaDataCache::deleteObjectMetaData (uint64_t objectId){
	mm::cache_map<uint64_t, ObjectMetaData*>::iterator _it = _objectMetaDataMap.find(objectId);
	if (_it != _objectMetaDataMap.end()) {
		_objectMetaDataMap.erase(_it->first);
	}
	//TODO cache missing
}

char* ObjectMetaDataCache::read(uint64_t id){
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	return (char *)readObjectMetaData(id);
}

void ObjectMetaDataCache::write(uint64_t id, char * data){
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	writeObjectMetaData(id,(ObjectMetaData *)data);
}

void ObjectMetaDataCache::deleteEntry(uint64_t id){
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	deleteaObjectMetaData(id);
}
