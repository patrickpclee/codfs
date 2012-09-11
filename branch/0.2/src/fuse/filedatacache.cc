#include "filedatacache.hh"

#include <openssl/md5.h>

#include "../client/client_communicator.hh"

#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../common/convertor.hh"

extern ClientCommunicator* _clientCommunicator;
extern string codingSetting;
extern CodingScheme codingScheme;
extern uint32_t _clientId;

FileDataCache::FileDataCache (struct FileMetaData fileMetaData, uint64_t objectSize)
	: _objectSize(objectSize),
	_metaData(fileMetaData)
{
	for(uint32_t i = 0; i < fileMetaData._objectList.size(); ++i)
	{
		struct ObjectData tempObjectData;
		tempObjectData.info.objectId = fileMetaData._objectList[i];
		//tempObjectData.buf = MemoryPool::getInstance().poolMalloc(_objectSize);	
		tempObjectData.info.objectSize = _objectSize;
		_objectDataList.push_back(tempObjectData);	
		_objectStatusList.push_back(NEW);
	}
	_fileSize = 0;
	_fileId = fileMetaData._id;
	_primaryList = fileMetaData._primaryList;
	//_objectCount = fileMetaData._objectList.size();
	_lastObjectCount = 0;
}

int64_t FileDataCache::write(const void* buf, uint32_t size, uint64_t offset)
{
	uint32_t firstObjectToWrite = offset / _objectSize;
	uint32_t lastObjectToWrite = (offset + size) / _objectSize;
	if(((offset + size) % _objectSize) == 0)
		--lastObjectToWrite;

	uint64_t byteWritten = 0;
	uint64_t bufOffset = 0;
	uint32_t objectWriteOffset;
	uint32_t writeSize;
	while (lastObjectToWrite >= _objectStatusList.size()) {
		vector<struct ObjectMetaData> objectMetaDataList = _clientCommunicator->getNewObjectList(_clientId, _objectStatusList.size() / 2 + 1);	
		for(uint32_t i = 0; i < objectMetaDataList.size(); ++i){
			struct ObjectData tempObjectData;
			tempObjectData.info.objectId = objectMetaDataList[i]._id;
			tempObjectData.info.objectSize = _objectSize;
			_primaryList.push_back(objectMetaDataList[i]._primary);
			_objectDataList.push_back(tempObjectData);
			_objectStatusList.push_back(NEW);
		}
	}

	for(uint32_t i = firstObjectToWrite; i <= lastObjectToWrite; ++i)
	{
		objectWriteOffset = (offset + byteWritten) - (i * _objectSize);
		writeSize = min(min(size - byteWritten, _objectSize), ((i + 1) * _objectSize - (offset + byteWritten)));
		if(_objectStatusList[i] == NEW){
			_objectDataList[i].buf = MemoryPool::getInstance().poolMalloc(_objectSize);
			_objectStatusList[i] = DIRTY;
		}
		//char* bufPtr = (char*)_objectDataList[i].buf + objectWriteOffset;
		memcpy((char*)_objectDataList[i].buf + objectWriteOffset, (char*)buf + bufOffset, writeSize);
		bufOffset += writeSize;
		byteWritten += writeSize;
	}
	if(lastObjectToWrite > _lastObjectCount)
		_lastObjectCount = lastObjectToWrite;
	if((offset + size) > _fileSize)
		_fileSize = offset + size;

	//TODO: Ask For More Object ID if exceed Preallocated Number
	//if(lastObjectToWrite >= _objectStatusList.size()){
		//Ask For New
	//}
	
	return size;
}

FileDataCache::~FileDataCache ()
{
	debug("%s\n","Delete");
	struct ObjectData objectData;
	uint32_t primary;
	uint32_t osdSockfd;
	vector<uint64_t> objectList;
	_objectDataList.resize(_lastObjectCount + 1);
	unsigned char checksum[MD5_DIGEST_LENGTH];
	for (uint32_t i = 0; i < _lastObjectCount; ++i)
	{
		if(_objectStatusList[i] != DIRTY)
			continue;
		objectData = _objectDataList[i];
		objectList.push_back(objectData.info.objectId);
		primary = _primaryList[i];
		osdSockfd = _clientCommunicator->getSockfdFromId(primary);
		
		MD5((unsigned char*) objectData.buf, objectData.info.objectSize, checksum);
		_clientCommunicator->sendObject(_clientId, osdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
		_objectStatusList[i] = CLEAN;
	}
	objectData = _objectDataList[_lastObjectCount];
	objectList.push_back(objectData.info.objectId);
	objectData.info.objectSize = _fileSize % _objectSize;
	if((_fileSize != 0) && (objectData.info.objectSize == 0))
		objectData.info.objectSize = _objectSize;
	primary = _primaryList[_lastObjectCount];
	osdSockfd = _clientCommunicator->getSockfdFromId(primary);
	_clientCommunicator->saveFileSize(_clientId, _fileId, _fileSize); 
	_clientCommunicator->saveObjectList(_clientId, _fileId, objectList);
	MD5((unsigned char*) objectData.buf, objectData.info.objectSize, checksum);
	_clientCommunicator->sendObject(_clientId, osdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
}
