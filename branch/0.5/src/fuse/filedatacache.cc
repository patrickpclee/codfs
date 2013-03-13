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

	FileDataCache::FileDataCache (struct FileMetaData fileMetaData, uint64_t segmentSize)
: _segmentSize(segmentSize),
	_metaData(fileMetaData)
{
	debug_cyan("File ID %"PRIu32" Segment Size %" PRIu64,fileMetaData._id,segmentSize);
	for(uint32_t i = 0; i < fileMetaData._segmentList.size(); ++i)
	{
		struct SegmentData tempSegmentData;
		tempSegmentData.info.segmentId = fileMetaData._segmentList[i];
		//tempSegmentData.buf = MemoryPool::getInstance().poolMalloc(_segmentSize);	
		tempSegmentData.info.segmentSize = _segmentSize;
		_segmentDataList.push_back(tempSegmentData);	
		_segmentStatusList.push_back(NEW);
	}
	_fileSize = 0;
	_fileId = fileMetaData._id;
	_primaryList = fileMetaData._primaryList;
	//_segmentCount = fileMetaData._segmentList.size();
	_lastSegmentCount = 0;
	_lastWriteBackPos = 0;
	_clean = true;
}

int64_t FileDataCache::write(const void* buf, uint32_t size, uint64_t offset)
{
	_clean = false;
	uint32_t firstSegmentToWrite = offset / _segmentSize;
	uint32_t lastSegmentToWrite = (offset + size) / _segmentSize;
	if(((offset + size) % _segmentSize) == 0)
		--lastSegmentToWrite;

	uint64_t byteWritten = 0;
	uint64_t bufOffset = 0;
	uint32_t segmentWriteOffset;
	uint32_t writeSize;
	while (lastSegmentToWrite >= _segmentStatusList.size()) {
		vector<struct SegmentMetaData> segmentMetaDataList = _clientCommunicator->getNewSegmentList(_clientId, _segmentStatusList.size() / 2 + 1);	
		for(uint32_t i = 0; i < segmentMetaDataList.size(); ++i){
			struct SegmentData tempSegmentData;
			tempSegmentData.info.segmentId = segmentMetaDataList[i]._id;
			tempSegmentData.info.segmentSize = _segmentSize;
			_primaryList.push_back(segmentMetaDataList[i]._primary);
			_segmentDataList.push_back(tempSegmentData);
			_segmentStatusList.push_back(NEW);
		}
	}

	if(firstSegmentToWrite  > _lastWriteBackPos + 1){
#ifdef PARALLEL_TRANSFER
		_writetp.wait(_writePoolLimit);
		_writetp.schedule(boost::bind(&FileDataCache::writeBack,this,_lastWriteBackPos));
		//_writetp.schedule(boost::bind(writeBackThread,this,_lastWriteBackPos));
#else
		writeBack(_lastWriteBackPos);
#endif
		++_lastWriteBackPos;
	}

	for(uint32_t i = firstSegmentToWrite; i <= lastSegmentToWrite; ++i)
	{
		segmentWriteOffset = (offset + byteWritten) - (i * _segmentSize);
		writeSize = min(min(size - byteWritten, _segmentSize), ((i + 1) * _segmentSize - (offset + byteWritten)));
		if(_segmentStatusList[i] == NEW){
			_segmentDataList[i].buf = MemoryPool::getInstance().poolMalloc(_segmentSize);
			_segmentStatusList[i] = DIRTY;
		}
		//char* bufPtr = (char*)_segmentDataList[i].buf + segmentWriteOffset;
		memcpy((char*)_segmentDataList[i].buf + segmentWriteOffset, (char*)buf + bufOffset, writeSize);
		bufOffset += writeSize;
		byteWritten += writeSize;
	}
	if(lastSegmentToWrite > _lastSegmentCount)
		_lastSegmentCount = lastSegmentToWrite;
	if((offset + size) > _fileSize)
		_fileSize = offset + size;

	//TODO: Ask For More Segment ID if exceed Preallocated Number
	//if(lastSegmentToWrite >= _segmentStatusList.size()){
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
	struct SegmentData segmentData;
	uint32_t primary;
	uint32_t osdSockfd;
	vector<uint64_t> segmentList;
	_segmentDataList.resize(_lastSegmentCount + 1);
	for (uint32_t i = 0; i < _lastSegmentCount; ++i)
	{
		//if(_segmentStatusList[i] != DIRTY)
		//	continue;

#ifdef PARALLEL_TRANSFER
		_writetp.schedule(boost::bind(writeBackThread,this,i));
#else
		writeBack(i);
#endif
		segmentList.push_back(_segmentDataList[i].info.segmentId);
		/*
		   segmentData = _segmentDataList[i];
		   segmentList.push_back(segmentData.info.segmentId);
		   primary = _primaryList[i];
		   osdSockfd = _clientCommunicator->getSockfdFromId(primary);

		   MD5((unsigned char*) segmentData.buf, segmentData.info.segmentSize, checksum);
		   debug_cyan("Send Segment %" PRIu64 " Size %" PRIu64"\n",segmentData.info.segmentId,segmentData.info.segmentSize);
		   _clientCommunicator->sendSegment(_clientId, osdSockfd, segmentData, codingScheme, codingSetting, md5ToHex(checksum));
		   MemoryPool::getInstance().poolFree(segmentData.buf);
		   _segmentStatusList[i] = CLEAN;
		 */
	}
#ifdef PARALLEL_TRANSFER
	_writetp.wait();
#endif
	segmentData = _segmentDataList[_lastSegmentCount];
	segmentList.push_back(segmentData.info.segmentId);
	segmentData.info.segmentSize = _fileSize % _segmentSize;
	debug_cyan("Send Segment 1 %" PRIu64 " Size %" PRIu32" File Size %"PRIu64" Segment Size %"PRIu64"\n",segmentData.info.segmentId,segmentData.info.segmentSize,_fileSize,_segmentSize);
	if((_fileSize != 0) && (segmentData.info.segmentSize == 0))
		segmentData.info.segmentSize = _segmentSize;
	debug_cyan("Send Segment 2 %" PRIu64 " Size %" PRIu32"\n",segmentData.info.segmentId,segmentData.info.segmentSize);
	primary = _primaryList[_lastSegmentCount];
	osdSockfd = _clientCommunicator->getSockfdFromId(primary);

	unsigned char checksum[MD5_DIGEST_LENGTH];
    memset (checksum, 0, MD5_DIGEST_LENGTH);

#ifdef USE_CHECKSUM
	MD5((unsigned char*) segmentData.buf, segmentData.info.segmentSize, checksum);
#endif

	debug_cyan("Send Segment 3 %" PRIu64 " Size %" PRIu32"\n",segmentData.info.segmentId,segmentData.info.segmentSize);
	_clientCommunicator->sendSegment(_clientId, osdSockfd, segmentData, codingScheme, codingSetting, md5ToHex(checksum));
	MemoryPool::getInstance().poolFree(segmentData.buf);
	_segmentStatusList[_lastSegmentCount] = CLEAN;
	_clientCommunicator->saveFileSize(_clientId, _fileId, _fileSize); 
	_clientCommunicator->saveSegmentList(_clientId, _fileId, segmentList);
	_clean = true;
}

void FileDataCache::writeBack(uint32_t index) {
	{
		lock_guard<mutex> lk(_writeBackMutex);
		if(_segmentStatusList[index] != DIRTY)
			return;
		_segmentStatusList[index] = PROCESSING;
	}
	debug_cyan("Write Back Segment at Index %" PRIu32 " [%" PRIu64 "]\n",index,_segmentDataList[index].info.segmentId);
	struct SegmentData segmentData = _segmentDataList[index];
	uint32_t primary = _primaryList[index];
	uint32_t osdSockfd = _clientCommunicator->getSockfdFromId(primary);

	unsigned char checksum[MD5_DIGEST_LENGTH];
    memset (checksum, 0, MD5_DIGEST_LENGTH);

#ifdef USE_CHECKSUM
	MD5((unsigned char*) segmentData.buf, segmentData.info.segmentSize, checksum);
#endif

	debug_cyan("Send Segment %" PRIu64 " Size %" PRIu32"\n",segmentData.info.segmentId,segmentData.info.segmentSize);
	_clientCommunicator->sendSegment(_clientId, osdSockfd, segmentData, codingScheme, codingSetting, md5ToHex(checksum));
	MemoryPool::getInstance().poolFree(segmentData.buf);
	_segmentStatusList[index] = CLEAN;
}
