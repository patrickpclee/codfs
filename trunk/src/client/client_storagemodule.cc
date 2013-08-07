#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <sys/file.h>
#include <thread>
#include <mutex>	// added for gcc 4.7.1
#include <unistd.h>	// added for gcc 4.7.1
#include "../common/debug.hh"
#include "../common/memorypool.hh"
#include "client_storagemodule.hh"
#include "../config/config.hh"
using namespace std;

/// Config Layer
extern ConfigLayer* configLayer;

// Global Mutex for locking file during read / write
mutex fileMutex;
mutex transferCacheMutex;
mutex openedFileMutex;

ClientStorageModule::ClientStorageModule() {
	// read config value

	_segmentCache = {};
	_segmentSize = configLayer->getConfigLong("Storage>SegmentSize") * 1024;
	debug("Config Segment Size = %" PRIu64 " Bytes\n", _segmentSize);
}

uint64_t ClientStorageModule::getFilesize(string filepath) {

	ifstream in(filepath, ifstream::in | ifstream::binary);

	if (!in) {
		debug("%s\n", "ERROR: Cannot open file");
		exit(-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	in.close();

	return filesize;

}

uint32_t ClientStorageModule::getSegmentCount(string filepath) {

	const uint64_t filesize = getFilesize(filepath);

	if (filesize == 0) {
		return 0;
	}

	uint32_t segmentCount = (uint32_t) ((filesize - 1) / _segmentSize + 1);

	return segmentCount;
}

struct SegmentData ClientStorageModule::readSegmentFromFile(string filepath,
		uint32_t segmentIndex) {

	// TODO: now assume that client only do one I/O function at a time
	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	struct SegmentData segmentData;
	uint32_t byteToRead = 0;

	// index starts from 0
	const uint64_t offset = segmentIndex * _segmentSize;

	FILE* file = fopen(filepath.c_str(), "rb");

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot open");
		exit(-1);
	}

	// Read Lock
	if (flock(fileno(file), LOCK_SH) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_SH");
		exit(-1);
	}

	// check filesize
	fseek(file, 0, SEEK_END);
	uint64_t filesize = ftell(file);

	if (offset >= filesize) {
		debug("%s\n", "ERROR: offset exceeds filesize");
		exit(-1);
	}

	if (filesize - offset > _segmentSize) {
		byteToRead = _segmentSize;
	} else {
		byteToRead = filesize - offset;
	}

	// Read file contents into buffer
	segmentData.buf = MemoryPool::getInstance().poolMalloc(byteToRead);
	uint32_t byteRead = pread(fileno(file), segmentData.buf, byteToRead, offset);

	// Release lock
	if (flock(fileno(file), LOCK_UN) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_UN");
		exit(-1);
	}

	if (byteRead != byteToRead) {
		debug("ERROR: Length = %" PRIu32 ", byteRead = %" PRIu32 "\n",
				byteToRead, byteRead);
		exit(-1);
	}

	fclose(file);

	// fill in information about segment
	segmentData.info.segLength = byteToRead;

	return segmentData;

}

void ClientStorageModule::setSegmentCache (uint64_t segmentId, SegmentData segmentCache) {
	{
		lock_guard<mutex> lk(transferCacheMutex);
		_segmentCache[segmentId] = segmentCache;
	}
}

void ClientStorageModule::createSegmentCache(uint64_t segmentId,
		uint32_t segLength, uint32_t bufLength) {

	// create cache
	struct SegmentData segmentCache;
	segmentCache.info.segmentId = segmentId;
	segmentCache.info.segLength = segLength;
	segmentCache.bufLength = bufLength;
	segmentCache.buf = MemoryPool::getInstance().poolMalloc(segLength);

	// save cache to map
	{
		lock_guard<mutex> lk(transferCacheMutex);
		_segmentCache[segmentId] = segmentCache;
	}

	debug("Segment created ID = %" PRIu64 " segLength = %" PRIu32 " bufLength = %" PRIu32 "\n",
			segmentId, segLength, bufLength);
}

uint32_t ClientStorageModule::writeSegmentCache(uint64_t segmentId, char* buf,
		uint64_t offsetInSegment, uint32_t length) {

	char* recvCache;

	{
		lock_guard<mutex> lk(transferCacheMutex);
		if (!_segmentCache.count(segmentId)) {
			debug("%s\n", "cannot find cache for segment");
			exit(-1);
		}
		recvCache = _segmentCache[segmentId].buf;
	}

	memcpy(recvCache + offsetInSegment, buf, length);

	return length;
}

bool ClientStorageModule::locateSegmentCache(uint64_t segmentId){
	lock_guard<mutex> lk(transferCacheMutex);
	return _segmentCache.count(segmentId);
}

struct SegmentData ClientStorageModule::getSegmentCache(uint64_t segmentId) {
	lock_guard<mutex> lk(transferCacheMutex);
	if (!_segmentCache.count(segmentId)) {
		debug("segment cache not found for %" PRIu64 "\n", segmentId);
		exit(-1);
	}
	return _segmentCache[segmentId];
}

void ClientStorageModule::closeSegment(uint64_t segmentId) {

	// close cache
	if(_segmentCache.count(segmentId) == 0){
		debug_yellow("Segment Cache Not Found for %" PRIu64 "\n", segmentId);
		return;
	}
	struct SegmentData segmentCache = getSegmentCache(segmentId);
	MemoryPool::getInstance().poolFree(segmentCache.buf);

	{
		lock_guard<mutex> lk(transferCacheMutex);
		_segmentCache.erase(segmentId);
	}

	debug("Segment ID = %" PRIu64 " closed\n", segmentId);
}

FILE* ClientStorageModule::createAndOpenFile(string filepath) {
	// open file for read/write
	// create new if not exist
	FILE* filePtr;
	filePtr = fopen(filepath.c_str(), "wb+");

	if (filePtr == NULL) {
		debug("%s\n", "Unable to create file!");
		return NULL;
	}

	// set buffer to zero to avoid memory leak
	setvbuf(filePtr, NULL, _IONBF, 0);

	return filePtr;

}

uint32_t ClientStorageModule::writeFile(FILE* file, string filepath, char* buf,
		uint64_t offset, uint32_t length) {

	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot write");
		exit(-1);
	}

	// Write file contents from buffer
	uint32_t byteWritten = pwrite(fileno(file), buf, length, offset);

	if (byteWritten != length) {
		debug("ERROR: Length = %d, byteWritten = %d\n", length, byteWritten);
		exit(-1);
	}

	return byteWritten;
}

void ClientStorageModule::closeFile (FILE* filePtr) {
	fclose (filePtr);
}

uint64_t ClientStorageModule::getSegmentSize() {
	return _segmentSize;
}
