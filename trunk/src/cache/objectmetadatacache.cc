#include <thread>
#include <sstream>
#include "objectmetadatacache.hh"

std::mutex objectCacheMutex;

ObjectMetaDataCache::ObjectMetaDataCache(){
	_objectMetaDataMap.clear();
}

ObjectMetaDataCache::~ObjectMetaDataCache(){
	_objectMetaDataMap.clear();
}

ObjectMetaData* ObjectMetaDataCache::readObjectMetaData(std::string objectId){
	mm::cache_map<std::string, ObjectMetaData*>::iterator _it = _objectMetaDataMap.find(objectId);
	if (_it != _objectMetaDataMap.end()) {
		return _it->second;
	}
	return NULL;
	//TODO cache missing
}

void ObjectMetaDataCache::writeObjectMetaData (std::string objectId, ObjectMetaData *objectMetaData){
	_objectMetaDataMap.insert(objectId,objectMetaData);
}

void ObjectMetaDataCache::deleteObjectMetaData (std::string objectId){
	mm::cache_map<std::string, ObjectMetaData*>::iterator _it = _objectMetaDataMap.find(objectId);
	if (_it != _objectMetaDataMap.end()) {
		_objectMetaDataMap.erase(_it->first);
	}
	//TODO cache missing
}

char* ObjectMetaDataCache::read(uint64_t id){
	std::ostringstream ss;
	ss << id;
	std::string sid = ss.str();;
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	return (char *)readObjectMetaData(sid);
}

void ObjectMetaDataCache::write(uint64_t id, char * data){
	std::ostringstream ss;
	ss << id;
	std::string sid = ss.str();;
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	writeObjectMetaData(sid,(ObjectMetaData *)data);
}

void ObjectMetaDataCache::deleteEntry(uint64_t id){
	std::ostringstream ss;
	ss << id;
	std::string sid = ss.str();;
	std::lock_guard<std::mutex> lk(objectCacheMutex);
	deleteObjectMetaData(sid);
}

