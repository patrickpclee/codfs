#include "filemetadatacache.hh"

uint32_t FileMetaDataCache::path2Id(string path) {
	return _fileIdCache.at(path);
}
string FileMetaDataCache::id2Path(uint32_t id) {
	return _metaDataCache.at(id)._path;
}

struct FileMetaData& FileMetaDataCache::getMetaData(uint32_t id) {
	return _metaDataCache.at(id);
}

void FileMetaDataCache::saveMetaData(const struct FileMetaData& fileMetaData) {
	uint32_t id = fileMetaData._id;
	string path = fileMetaData._path;
	_fileIdCache[path] = id;
	_metaDataCache[id] = fileMetaData;
	return ;
}

void FileMetaDataCache::removeMetaData(uint32_t id) {
	string path = id2Path(id);
	_fileIdCache.erase(path);
	_metaDataCache.erase(id);
	return ;
}
