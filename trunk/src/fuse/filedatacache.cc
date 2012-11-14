#include "filedatacache.hh"

#include <openssl/md5.h>
#include "string.h"

#include "../client/client_communicator.hh"

#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../common/convertor.hh"

extern ClientCommunicator* _clientCommunicator;
extern string codingSetting;
extern CodingScheme codingScheme;
extern uint32_t _clientId;

#ifdef PARALLEL_TRANSFER

#include "../../lib/threadpool/threadpool.hpp"
extern boost::threadpool::pool _writetp;
extern uint32_t _writePoolLimit;

void writeBackThread(FileDataCache* fileDataCache, uint32_t index){
	fileDataCache->writeBack(index);
}
#endif

	FileDataCache::FileDataCache (struct FileMetaData fileMetaData, uint64_t objectSize)
: _objectSize(objectSize),
	_metaData(fileMetaData)
{
	debug_cyan("File ID %"PRIu32" Object Size %" PRIu64,fileMetaData._id,objectSize);
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
	_lastWriteBackPos = 0;
	_clean = true;
}

int64_t FileDataCache::write(const void* buf, uint32_t size, uint64_t offset)
{
	_clean = false;
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

	if(firstObjectToWrite  > _lastWriteBackPos + 1){
#ifdef PARALLEL_TRANSFER
		_writetp.wait(_writePoolLimit);
		_writetp.schedule(boost::bind(&FileDataCache::writeBack,this,_lastWriteBackPos));
		//_writetp.schedule(boost::bind(writeBackThread,this,_lastWriteBackPos));
#else
		writeBack(_lastWriteBackPos);
#endif
		++_lastWriteBackPos;
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
	flush();
}

void FileDataCache::flush(){
	if(_clean)
		return ;
	struct ObjectData objectData;
	uint32_t primary;
	uint32_t osdSockfd;
	vector<uint64_t> objectList;
	_objectDataList.resize(_lastObjectCount + 1);
	unsigned char checksum[MD5_DIGEST_LENGTH];
	for (uint32_t i = 0; i < _lastObjectCount; ++i)
	{
		//if(_objectStatusList[i] != DIRTY)
		//	continue;

#ifdef PARALLEL_TRANSFER
		_writetp.schedule(boost::bind(writeBackThread,this,i));
#else
		writeBack(i);
#endif
		objectList.push_back(_objectDataList[i].info.objectId);
		/*
		   objectData = _objectDataList[i];
		   objectList.push_back(objectData.info.objectId);
		   primary = _primaryList[i];
		   osdSockfd = _clientCommunicator->getSockfdFromId(primary);

		   MD5((unsigned char*) objectData.buf, objectData.info.objectSize, checksum);
		   debug_cyan("Send Object %" PRIu64 " Size %" PRIu64"\n",objectData.info.objectId,objectData.info.objectSize);
		   _clientCommunicator->sendObject(_clientId, osdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
		   MemoryPool::getInstance().poolFree(objectData.buf);
		   _objectStatusList[i] = CLEAN;
		 */
	}
#ifdef PARALLEL_TRANSFER
	_writetp.wait();
#endif
	objectData = _objectDataList[_lastObjectCount];
	objectList.push_back(objectData.info.objectId);
	objectData.info.objectSize = _fileSize % _objectSize;
	debug_cyan("Send Object 1 %" PRIu64 " Size %" PRIu32" File Size %"PRIu64" Object Size %"PRIu64"\n",objectData.info.objectId,objectData.info.objectSize,_fileSize,_objectSize);
	if((_fileSize != 0) && (objectData.info.objectSize == 0))
		objectData.info.objectSize = _objectSize;
	debug_cyan("Send Object 2 %" PRIu64 " Size %" PRIu32"\n",objectData.info.objectId,objectData.info.objectSize);
	primary = _primaryList[_lastObjectCount];
	osdSockfd = _clientCommunicator->getSockfdFromId(primary);
	MD5((unsigned char*) objectData.buf, objectData.info.objectSize, checksum);
	debug_cyan("Send Object 3 %" PRIu64 " Size %" PRIu32"\n",objectData.info.objectId,objectData.info.objectSize);
	_clientCommunicator->sendObject(_clientId, osdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
	MemoryPool::getInstance().poolFree(objectData.buf);
	_objectStatusList[_lastObjectCount] = CLEAN;
	_clientCommunicator->saveFileSize(_clientId, _fileId, _fileSize); 
	_clientCommunicator->saveObjectList(_clientId, _fileId, objectList);
	_clean = true;
}

void FileDataCache::writeBack(uint32_t index) {
	{
		lock_guard<mutex> lk(_writeBackMutex);
		if(_objectStatusList[index] != DIRTY)
			return;
		_objectStatusList[index] = PROCESSING;
	}
	debug_cyan("Write Back Object at Index %" PRIu32 " [%" PRIu64 "]\n",index,_objectDataList[index].info.objectId);
	struct ObjectData objectData = _objectDataList[index];
	uint32_t primary = _primaryList[index];
	uint32_t osdSockfd = _clientCommunicator->getSockfdFromId(primary);
	unsigned char checksum[MD5_DIGEST_LENGTH];
	MD5((unsigned char*) objectData.buf, objectData.info.objectSize, checksum);
	debug_cyan("Send Object %" PRIu64 " Size %" PRIu32"\n",objectData.info.objectId,objectData.info.objectSize);
	_clientCommunicator->sendObject(_clientId, osdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
	MemoryPool::getInstance().poolFree(objectData.buf);
	_objectStatusList[index] = CLEAN;
}
