#include "filedatacache.hh"

FileDataCache::FileDataCache (struct FileMetaData fileMetaData, uint64_t objectSize)
	: _metaData(fileMetaData),
	  _objetcSize(objectSize)
{
}

int64_t FileDataCache::write(void* buf, uint32_t size, uint64_t offset)
{
	uint32_t fistObjectToWrite = offset / _objectSize;
	uint32_t lastObjectToWrite = (offset + size) / _objectSize;
	if(((offset + size) % _objectSize) == 0)
		--lastObjectToWrite;
	if(lastObjectToWrite >= _objectStatusList.size()){
		//Ask For New
	}
}

FileDataCache::~FileDataCache ()
{

}
