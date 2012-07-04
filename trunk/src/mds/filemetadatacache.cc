#include "filemetadatacache.hh"

uint32_t FileMetaDataCache::readFileMetaData (uint32_t fileId){
	throw CacheMiss;
}

uint32_t FileMetaDataCache::writeFileMetaData (uint32_t fileId, FileMetaData fileMetaData){
	throw CacheMiss;
}

uint32_t FileMetaDataCache::createFileMetaData (uint32_t fileId){
	return 0;
}

uint32_t FileMetaDataCache::deleteFileMetaData (uint32_t fileId){
	return 0;
}
