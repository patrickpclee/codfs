#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <sys/file.h>
#include <thread>
#include "../common/debug.hh"
#include "../common/memorypool.hh"
#include "client_storagemodule.hh"
#include "../config/config.hh"
using namespace std;

/// Config Layer
extern ConfigLayer* configLayer;

// Global Mutex for locking file during read / write
mutex fileMutex;
mutex cacheMutex;
mutex openedFileMutex;

ClientStorageModule::ClientStorageModule() {
	// read config value

	_objectCache = {};
	_openedFile = {};
//	_objectFolder = configLayer->getConfigString("Storage>ObjectLocation");
	_objectSize = configLayer->getConfigLong("Storage>ObjectSize") * 1024;
	debug("Config Object Size = %" PRIu64 " Bytes\n", _objectSize);
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

uint32_t ClientStorageModule::getObjectCount(string filepath) {

	const uint64_t filesize = getFilesize(filepath);

	if (filesize == 0) {
		return 0;
	}

	uint32_t objectCount = (uint32_t) ((filesize - 1) / _objectSize + 1);

	return objectCount;
}

struct ObjectData ClientStorageModule::readObjectFromFile(string filepath,
		uint32_t objectIndex) {

	// TODO: now assume that client only do one I/O function at a time
	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	struct ObjectData objectData;
	uint32_t byteToRead = 0;

	// index starts from 0
	const uint64_t offset = objectIndex * _objectSize;

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

	if (filesize - offset > _objectSize) {
		byteToRead = _objectSize;
	} else {
		byteToRead = filesize - offset;
	}

	// Read file contents into buffer
	// poolFree in ClientCommunicator::putObject
	objectData.buf = MemoryPool::getInstance().poolMalloc(byteToRead);
	uint32_t byteRead = pread(fileno(file), objectData.buf, byteToRead, offset);

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

	// fill in information about object
	objectData.info.objectSize = byteToRead;

	return objectData;

}

void ClientStorageModule::writeObjectToFile(string dstPath, struct ObjectData objectData, uint32_t objectIndex) {

	// TODO: now assume that client only do one I/O function at a time
	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	const uint32_t byteToWrite = objectData.info.objectSize;
	const uint64_t offset = objectIndex * _objectSize;

	FILE* file = fopen(dstPath.c_str(), "wb");

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot open");
		exit(-1);
	}

	// Write Lock
	if (flock(fileno(file), LOCK_EX) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_SH");
		exit(-1);
	}

	uint32_t byteWritten = pwrite (fileno(file), objectData.buf, byteToWrite, offset);

	// Release lock
	if (flock(fileno(file), LOCK_UN) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_UN");
		exit(-1);
	}

	if (byteWritten != byteToWrite) {
		debug("ERROR: Length = %" PRIu32 ", byteWritten = %" PRIu32 "\n",
				byteToWrite, byteWritten);
		exit(-1);
	}

	fclose(file);

}

void ClientStorageModule::createObject(uint64_t objectId, uint32_t length) {

	// create cache
	struct ObjectCache objectCache;
	objectCache.length = length;
	objectCache.buf = MemoryPool::getInstance().poolMalloc(length);

	// save cache to map
	{
		lock_guard<mutex> lk(cacheMutex);
		_objectCache[objectId] = objectCache;
	}

	// create object
	createAndOpenObject(objectId, length);

	// write info
	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);

	debug("Object created ID = %" PRIu64 " Length = %" PRIu32 " Path = %s\n",
			objectId, length, filepath.c_str());
}

FILE* ClientStorageModule::createAndOpenObject(uint64_t objectId, uint32_t length) {

	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);

	debug("Object ID = %" PRIu64 " created\n", objectId);

	return createFile(filepath);
}

void ClientStorageModule::writeObjectInfo(uint64_t objectId, uint32_t objectSize,
		string filepath) {

	// TODO: Database to be implemented

}

FILE* ClientStorageModule::createFile(string filepath) {

	// open file for read/write
	// create new if not exist
	FILE* filePtr;
	filePtr = fopen(filepath.c_str(), "wb+");

	// set buffer to zero to avoid memory leak
	setvbuf(filePtr, NULL, _IONBF, 0);

	debug("fileptr = %p\n", filePtr);

	if (filePtr == NULL) {
		debug("%s\n", "Unable to create file!");
		return NULL;
	}

	// add file pointer to map
	openedFileMutex.lock();
	_openedFile[filepath] = filePtr;
	openedFileMutex.unlock();

	return filePtr;
}

uint32_t ClientStorageModule::writeObjectCache(uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length) {

	char* recvCache;

	{
		lock_guard<mutex> lk(cacheMutex);
		if (!_objectCache.count(objectId)) {
			debug("%s\n", "cannot find cache for object");
			exit(-1);
		}
		recvCache = _objectCache[objectId].buf;
	}

	memcpy(recvCache + offsetInObject, buf, length);

	return length;
}

struct ObjectCache ClientStorageModule::getObjectCache(uint64_t objectId) {
	lock_guard<mutex> lk(cacheMutex);
	if (!_objectCache.count(objectId)) {
		debug("%s\n", "object cache not found");
		exit(-1);
	}
	return _objectCache[objectId];
}

string ClientStorageModule::writeObject(uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length) {

	uint32_t byteWritten;

	string filepath = generateObjectPath(objectId, _objectFolder);
	byteWritten = writeFile(filepath, buf, offsetInObject, length);

	debug(
			"Object ID = %" PRIu64 " write %" PRIu32 " bytes at offset %" PRIu64 "\n",
			objectId, byteWritten, offsetInObject);

	return filepath;
}

void ClientStorageModule::closeObject(uint64_t objectId) {
	string filepath = generateObjectPath(objectId, _objectFolder);
	closeFile(filepath);

	// close cache
	struct ObjectCache objectCache = getObjectCache(objectId);
	MemoryPool::getInstance().poolFree(objectCache.buf);

	{
		lock_guard<mutex> lk(cacheMutex);
		_objectCache.erase(objectId);
	}

	debug("Object ID = %" PRIu64 " closed\n", objectId);
}

string ClientStorageModule::generateObjectPath(uint64_t objectId, string objectFolder) {

	// append a '/' if not present
	if (objectFolder[objectFolder.length() - 1] != '/') {
		objectFolder.append("/");
	}

	return objectFolder + to_string(objectId);
}

uint32_t ClientStorageModule::writeFile(string filepath, char* buf, uint64_t offset, uint32_t length) {


	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	FILE* file = openFile(filepath);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot write");
		exit(-1);
	}

	/*

	// Write Lock
	if (flock(fileno(file), LOCK_EX) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_EX");
		exit(-1);
	}

	*/

	// Write file contents from buffer

	uint32_t byteWritten = pwrite(fileno(file), buf, length, offset);

	/*
	fseek (file, offset, SEEK_SET);
	uint32_t byteWritten = fwrite (buf, 1, length, file);
	fflush (file);
	*/

	/*

	// Release lock
	if (flock(fileno(file), LOCK_UN) == -1) {
		debug("%s\n", "ERROR: Cannot LOCK_UN");
		exit(-1);
	}

	*/

	if (byteWritten != length) {
		debug("ERROR: Length = %d, byteWritten = %d\n", length, byteWritten);
		exit(-1);
	}

	return byteWritten;
}

void ClientStorageModule::closeFile(string filepath) {

	FILE* filePtr = openFile(filepath);

	openedFileMutex.lock();
	_openedFile.erase(filepath);
	openedFileMutex.unlock();

	fclose(filePtr);
}

FILE* ClientStorageModule::openFile(string filepath) {

	openedFileMutex.lock();

	// find file in map
	if (_openedFile.count(filepath)) {
		FILE* openedFile = _openedFile[filepath];
		openedFileMutex.unlock();
		return openedFile;
	}

	openedFileMutex.unlock();

	FILE* filePtr;
	filePtr = fopen(filepath.c_str(), "rb+");

	// set buffer to zero to avoid memory leak
	setvbuf (filePtr, NULL , _IONBF , 0);

	if (filePtr == NULL) {
		debug("Unable to open file at %s\n", filepath.c_str());
		return NULL;
	}

	// add file pointer to map

	openedFileMutex.lock();
	_openedFile[filepath] = filePtr;
	openedFileMutex.unlock();

	return filePtr;
}
