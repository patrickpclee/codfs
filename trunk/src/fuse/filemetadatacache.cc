#include <stdexcept>      // std::out_of_range

#include "filemetadatacache.hh"

#include "../common/debug.hh"

uint32_t FileMetaDataCache::path2Id(string path) {
    readLock rdlock(_metaDataCacheMutex);
	try {
		return _fileIdCache.at(path);
	} catch (const std::out_of_range& oor) {
		return 0;
	}
}
string FileMetaDataCache::id2Path(uint32_t id) {
    readLock rdlock(_metaDataCacheMutex);
	return _metaDataCache.at(id)._path;
}

struct FileMetaData& FileMetaDataCache::getMetaData(uint32_t id) {
    readLock rdlock(_metaDataCacheMutex);
	return _metaDataCache.at(id);
}

void FileMetaDataCache::saveMetaData(const struct FileMetaData& fileMetaData) {
    writeLock wtlock(_metaDataCacheMutex);
	uint32_t id = fileMetaData._id;
	string path = fileMetaData._path;
	_fileIdCache[path] = id;
	_metaDataCache[id] = fileMetaData;
	return ;
}

void FileMetaDataCache::removeMetaData(uint32_t id) {
    writeLock wtlock(_metaDataCacheMutex);
	string path = id2Path(id);
	{
		try{
			_fileIdCache.erase(path);
			_metaDataCache.erase(id);
		} catch (const std::out_of_range& oor) {
			debug("File %s [%" PRIu32 "] Not in Meta Data Cache\n",path.c_str(),id);
			return ;
		}
	}
	return ;
}

int FileMetaDataCache::renameMetaData(string path, string new_path) {
    uint32_t id = 0;
    {
        readLock rdlock(_metaDataCacheMutex);
	    id = path2Id(path);
        if (id == 0) {
		    debug_error("Metadata Not Found %s\n",path.c_str());
		    return -1;
	    }
    }
	struct FileMetaData &fileMetaData = getMetaData(id);
    {
        writeLock wtlock(_metaDataCacheMutex);
	    fileMetaData._path = new_path;
	    _fileIdCache.erase(path);
	    _fileIdCache[new_path] = id;
	    _metaDataCache[id] = fileMetaData;
    }
	return 0;
}
