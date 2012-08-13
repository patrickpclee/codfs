#include <thread>
#include "filemetadatacache.hh"

std::mutex fileCacheMutex;

FileMetaDataCache::FileMetaDataCache(){
	_fileMetaDataMap.clear();
}

FileMetaDataCache::~FileMetaDataCache(){
	_fileMetaDataMap.clear();
}

FileMetaData* FileMetaDataCache::readFileMetaData(uint32_t fileId){
	mm::cache_map<uint32_t, FileMetaData*>::iterator _it = _fileMetaDataMap.find(fileId);
	if (_it != _fileMetaDataMap.end()) {
		return _it->second;
	}
	return NULL;
	//TODO cache missing
}

void FileMetaDataCache::writeFileMetaData (uint32_t fileId, FileMetaData *fileMetaData){
	_fileMetaDataMap.insert(fileId,fileMetaData);
}

void FileMetaDataCache::deleteFileMetaData (uint32_t fileId){
	mm::cache_map<uint32_t, FileMetaData*>::iterator _it = _fileMetaDataMap.find(fileId);
	if (_it != _fileMetaDataMap.end()) {
		_fileMetaDataMap.erase(_it->first);
		return;
	}
	//TODO cache missing
}

char* FileMetaDataCache::read(uint64_t id){
	std::lock_guard<std::mutex> lk(fileCacheMutex);
	return (char *)readFileMetaData(id);
}

void FileMetaDataCache::write(uint64_t id, char * data){
	std::lock_guard<std::mutex> lk(fileCacheMutex);
	writeFileMetaData(id,(FileMetaData *)data);
}

void FileMetaDataCache::deleteEntry(uint64_t id){
	std::lock_guard<std::mutex> lk(fileCacheMutex);
	deleteFileMetaData(id);
}

