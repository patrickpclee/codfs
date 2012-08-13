#include <thread>
#include "osdmetadatacache.hh"

std::mutex osdCacheMutex;

OsdMetaDataCache::OsdMetaDataCache(){
	_osdMetaDataMap.clear();
}

OsdMetaDataCache::~OsdMetaDataCache(){
	_osdMetaDataMap.clear();
}

SegmentMetaData* OsdMetaDataCache::readOsdMetaData(uint32_t osdId){
	mm::cache_map<uint32_t, SegmentMetaData*>::iterator _it = _osdMetaDataMap.find(osdId);
	if (_it != _osdMetaDataMap.end()) {
		return _it->second;
	}
	return NULL;
	//TODO cache missing
}

void OsdMetaDataCache::writeOsdMetaData (uint32_t osdId, SegmentMetaData *osdMetaData){
	_osdMetaDataMap.insert(osdId,osdMetaData);
}

void OsdMetaDataCache::deleteOsdMetaData (uint32_t osdId){
	mm::cache_map<uint32_t, SegmentMetaData*>::iterator _it = _osdMetaDataMap.find(osdId);
	if (_it != _osdMetaDataMap.end()) {
		_osdMetaDataMap.erase(_it->first);
	}
	//TODO cache missing
}

char* OsdMetaDataCache::read(uint64_t id){
	std::lock_guard<std::mutex> lk(osdCacheMutex);
	return (char *)readOsdMetaData(id);
}

void OsdMetaDataCache::write(uint64_t id, char * data){
	std::lock_guard<std::mutex> lk(osdCacheMutex);
	writeOsdMetaData(id,(SegmentMetaData *)data);
}

void OsdMetaDataCache::deleteEntry(uint64_t id){
	std::lock_guard<std::mutex> lk(osdCacheMutex);
	deleteOsdMetaData(id);
}
