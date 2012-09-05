#include "filedatamodule.hh"

#include "../config/config.hh"

extern ConfigLayer* configLayer;

using namespace std;

FileDataModule::FileDataModule()
{
	_objectSize = configLayer->getConfigLong("Storage>ObjectSize") * 1024;
}

FileDataCache* FileDataModule::createFileDataCache (struct FileMetaData fileMetaData)
{
	uint32_t fileId = fileMetaData._id;

	lock_guard<mutex> lock(_fileReferenceCountMapMutex);
	if(_fileReferenceCountMap.count(fileId)){
		++_fileReferenceCountMap[fileId];
		return _fileDataCacheMap[fileId];
	} else {
		_fileReferenceCountMap[fileId] = 1;
		_fileDataCacheMap[fileId] = new FileDataCache(fileMetaData, _objectSize);
		return _fileDataCacheMap[fileId];
	}

	return NULL;
}

void FileDataModule::closeFileDataCache (uint32_t fileId)
{
	lock_guard<mutex> lock(_fileReferenceCountMapMutex);
	--_fileReferenceCountMap[fileId];
	if(_fileReferenceCountMap[fileId] == 0) {
		_fileReferenceCountMap.erase(fileId);
		delete _fileDataCacheMap[fileId];
		_fileDataCacheMap.erase(fileId);
	}
}
