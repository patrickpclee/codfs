/*
 * storagemodule.cc
 */

#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include <sys/file.h>
#include "storagemodule.hh"
#include "../common/debug.hh"

// global variable defined in each component
extern ConfigLayer* configLayer;

mutex fileMutex;
mutex openedFileMutex;
mutex cacheMutex;

StorageModule::StorageModule() {
	_capacity = 0;
	_freespace = 0;
	_openedFile = {};
	_objectCache = {};
	_objectFolder = configLayer->getConfigString("Storage>ObjectLocation");
	_segmentFolder = configLayer->getConfigString("Storage>SegmentLocation");
}

StorageModule::~StorageModule() {

}

uint64_t StorageModule::getFilesize(string filepath) {

	ifstream in(filepath, ifstream::in | ifstream::binary);

	if (!in) {
		debug("ERROR: Cannot open file: %s\n", filepath.c_str());
		exit(-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	in.close();

	return filesize;

}

void StorageModule::createObject(uint64_t objectId, uint32_t length) {

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

void StorageModule::createSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {
	createAndOpenSegment(objectId, segmentId, length);

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	writeSegmentInfo(objectId, segmentId, length, filepath);

	debug(
			"Segment created ObjID = %" PRIu64 " SegmentID = %" PRIu32 " Length = %" PRIu32 " Path = %s\n",
			objectId, segmentId, length, filepath.c_str());
}

bool StorageModule::isObjectExist(uint64_t objectId) {

	// TEST OBJECT READ
	/*
	 ObjectInfo objectInfo = readObjectInfo(objectId);
	 if (objectInfo.objectSize == 0) {
	 return false;
	 }
	 */

	return true;
}

/**
 * Default length = 0 (read whole object)
 */

struct ObjectData StorageModule::readObject(uint64_t objectId,
		uint64_t offsetInObject, uint32_t length) {

	struct ObjectData objectData;
	objectData.info = readObjectInfo(objectId);

	// check num of bytes to read
	// if length = 0, read whole object
	uint32_t byteToRead;
	if (length == 0) {
		byteToRead = objectData.info.objectSize;
	} else {
		byteToRead = length;
	}

	// TODO: check maximum malloc size
	// poolFree in osd_communicator::sendObject
	objectData.buf = MemoryPool::getInstance().poolMalloc(byteToRead);

	readFile(objectData.info.objectPath, objectData.buf, offsetInObject,
			byteToRead);

	debug(
			"Object ID = %" PRIu64 " read %" PRIu32 " bytes at offset %" PRIu64 "\n",
			objectId, byteToRead, offsetInObject);

	return objectData;

}

struct SegmentData StorageModule::readSegment(uint64_t objectId,
		uint32_t segmentId, uint64_t offsetInSegment, uint32_t length) {

	struct SegmentData segmentData;
	string segmentPath = generateSegmentPath(objectId, segmentId,
			_segmentFolder);
	segmentData.info.objectId = objectId;
	segmentData.info.segmentId = segmentId;
	segmentData.info.segmentPath = segmentPath;

	// check num of bytes to read
	// if length = 0, read whole segment
	uint32_t byteToRead;
	if (length == 0) {
		const uint32_t filesize = getFilesize(segmentPath);
		byteToRead = filesize;
	} else {
		byteToRead = length;
	}

	segmentData.info.segmentSize = byteToRead;

	segmentData.buf = MemoryPool::getInstance().poolMalloc(byteToRead);

	readFile(segmentPath, segmentData.buf, offsetInSegment, byteToRead);

	debug(
			"Object ID = %" PRIu64 " Segment ID = %" PRIu32 " read %" PRIu32 " bytes at offset %" PRIu64 "\n",
			objectId, segmentId, byteToRead, offsetInSegment);

	return segmentData;
}

uint32_t StorageModule::writeObjectCache(uint64_t objectId, char* buf,
		uint64_t offsetInObject, uint32_t length) {

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

uint32_t StorageModule::writeObject(uint64_t objectId, char* buf,
		uint64_t offsetInObject, uint32_t length) {

	uint32_t byteWritten;

	string filepath = generateObjectPath(objectId, _objectFolder);
	byteWritten = writeFile(filepath, buf, offsetInObject, length);

	debug(
			"Object ID = %" PRIu64 " write %" PRIu32 " bytes at offset %" PRIu64 "\n",
			objectId, byteWritten, offsetInObject);

	return byteWritten;
}

uint32_t StorageModule::writeSegment(uint64_t objectId, uint32_t segmentId,
		char* buf, uint64_t offsetInSegment, uint32_t length) {

	uint32_t byteWritten = 0;

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	byteWritten = writeFile(filepath, buf, offsetInSegment, length);

	debug(
			"Object ID = %" PRIu64 " Segment ID = %" PRIu32 " write %" PRIu32 " bytes at offset %" PRIu64 "\n",
			objectId, segmentId, byteWritten, offsetInSegment);

	return byteWritten;
}

FILE* StorageModule::createAndOpenObject(uint64_t objectId, uint32_t length) {

	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);

	debug("Object ID = %" PRIu64 " created\n", objectId);

	return createFile(filepath);
}

FILE* StorageModule::createAndOpenSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	writeSegmentInfo(objectId, segmentId, length, filepath);

	debug("Object ID = %" PRIu64 " Segment ID = %" PRIu32 " created\n",
			objectId, segmentId);

	return createFile(filepath);
}

void StorageModule::closeObject(uint64_t objectId) {
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

void StorageModule::closeSegment(uint64_t objectId, uint32_t segmentId) {
	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	closeFile(filepath);

	debug("Object ID = %" PRIu64 " Segment ID = %" PRIu32 " closed\n",
			objectId, segmentId);
}

uint32_t StorageModule::getCapacity() {
	// change in capacity to be implemented
	return _capacity;
}

uint32_t StorageModule::getFreespace() {
	// change in capacity to be implemented
	return _freespace;
}

//
// PRIVATE METHODS
//

void StorageModule::writeObjectInfo(uint64_t objectId, uint32_t objectSize,
		string filepath) {

	// TODO: Database to be implemented

}

struct ObjectInfo StorageModule::readObjectInfo(uint64_t objectId) {
	// TODO: Database to be implemented
	struct ObjectInfo objectInfo;

	objectInfo.objectId = objectId;
	objectInfo.objectPath = generateObjectPath(objectId, _objectFolder);

	// TEST OBJECT READ
	objectInfo.objectSize = 26; // HARDCODE;

	return objectInfo;
}

void StorageModule::writeSegmentInfo(uint64_t objectId, uint32_t segmentId,
		uint32_t segmentSize, string filepath) {
	// TODO: Database to be implemented

}

/*
 struct SegmentInfo StorageModule::readSegmentInfo(uint64_t objectId,
 uint32_t segmentId) {
 // TODO: Database to be implemented
 struct SegmentInfo segmentInfo;

 return segmentInfo;
 }
 */

uint32_t StorageModule::readFile(string filepath, char* buf, uint64_t offset,
		uint32_t length) {

	debug("Read File :%s\n", filepath.c_str());

	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	FILE* file = openFile(filepath);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot read");
		exit(-1);
	}

	/*

	 // Read Lock
	 if (flock(fileno(file), LOCK_SH) == -1) {
	 debug("%s\n", "ERROR: Cannot LOCK_SH");
	 exit(-1);
	 }
	 */

	// Read file contents into buffer
	uint32_t byteRead = pread(fileno(file), buf, length, offset);

	/*
	 // Release lock
	 if (flock(fileno(file), LOCK_UN) == -1) {
	 debug("%s\n", "ERROR: Cannot LOCK_UN");
	 exit(-1);
	 }
	 */

	if (byteRead != length) {
		debug("ERROR: Length = %" PRIu32 ", byteRead = %" PRIu32 "\n",
				length, byteRead);
		exit(-1);
	}

	return byteRead;
}

uint32_t StorageModule::writeFile(string filepath, char* buf, uint64_t offset,
		uint32_t length) {

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

string StorageModule::generateObjectPath(uint64_t objectId,
		string objectFolder) {

	// append a '/' if not present
	if (objectFolder[objectFolder.length() - 1] != '/') {
		objectFolder.append("/");
	}

	return objectFolder + to_string(objectId);
}

string StorageModule::generateSegmentPath(uint64_t objectId, uint32_t segmentId,
		string segmentFolder) {

	// append a '/' if not present
	if (segmentFolder[segmentFolder.length() - 1] != '/') {
		segmentFolder.append("/");
	}

	return segmentFolder + to_string(objectId) + "." + to_string(segmentId);
}

/**
 * Create and open a new file
 */

FILE* StorageModule::createFile(string filepath) {

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

/**
 * Open an existing file, return pointer directly if file is already open
 */

FILE* StorageModule::openFile(string filepath) {

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
	setvbuf(filePtr, NULL, _IONBF, 0);

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

/**
 * Close file, remove from map
 */

void StorageModule::closeFile(string filepath) {

	FILE* filePtr = openFile(filepath);

	openedFileMutex.lock();
	_openedFile.erase(filepath);
	openedFileMutex.unlock();

	fclose(filePtr);
}

struct ObjectCache StorageModule::getObjectCache(uint64_t objectId) {
	lock_guard<mutex> lk(cacheMutex);
	if (!_objectCache.count(objectId)) {
		debug("%s\n", "object cache not found");
		exit(-1);
	}
	return _objectCache[objectId];
}
