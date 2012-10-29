/*
 * storagemodule.cc
 */

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "storagemodule.hh"
#include "../common/debug.hh"
#include "../common/define.hh"
#include "../common/convertor.hh"

// global variable defined in each component
extern ConfigLayer* configLayer;

mutex fileMutex;
mutex openedFileMutex;
mutex transferCacheMutex;
mutex diskCacheMutex;
mutex diskSpaceMutex;

StorageModule::StorageModule() {
	_openedFile = new FileLruCache <string, FILE*> (MAX_OPEN_FILES);
	_objectTransferCache = {};
	_objectFolder = configLayer->getConfigString("Storage>ObjectCacheLocation");
	_segmentFolder = configLayer->getConfigString("Storage>SegmentLocation");

	// create folder if not exist
	struct stat st;
	if(stat(_objectFolder.c_str(),&st) != 0) {
		debug ("%s does not exist, make directory automatically\n", _objectFolder.c_str());
		if (mkdir (_objectFolder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
			perror ("mkdir");
			exit (-1);
		}
	}
	if(stat(_segmentFolder.c_str(),&st) != 0) {
		debug ("%s does not exist, make directory automatically\n", _segmentFolder.c_str());
		if (mkdir (_segmentFolder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
			perror ("mkdir");
			exit (-1);
		}
	}

	// Unit in StorageModule: Bytes
	_maxObjectCache = stringToByte(configLayer->getConfigString("Storage>ObjectCacheCapacity"));
	_maxSegmentCapacity = stringToByte(configLayer->getConfigString("Storage>SegmentCapacity"));

	cout << "=== STORAGE ===" << endl;
	cout << "Object Cache Location = " << _objectFolder << " Size = "
	<< formatSize(_maxObjectCache) << endl;
	cout << "Segment Storage Location = " << _segmentFolder << " Size = "
	<< formatSize(_maxSegmentCapacity) << endl;
	cout << "===============" << endl;

	initializeStorageStatus();
}

StorageModule::~StorageModule() {

}

void StorageModule::initializeStorageStatus() {

	//
	// initialize objects
	//

	_freeObjectSpace = _maxObjectCache;
	_freeSegmentSpace = _maxSegmentCapacity;
	_currentObjectUsage = 0;
	_currentSegmentUsage = 0;

	struct dirent* dent;
	DIR* srcdir;

	srcdir = opendir(_objectFolder.c_str());
	if (srcdir == NULL) {
		perror("opendir");
		exit(-1);
	}

	cout << "===== List of Files =====" << endl;

	while ((dent = readdir(srcdir)) != NULL) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			continue;

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
			perror(dent->d_name);
			continue;
		}

		// save file info
		struct ObjectDiskCache objectDiskCache;
		uint64_t objectId = boost::lexical_cast<uint64_t>(dent->d_name);
		objectDiskCache.length = st.st_size;
		objectDiskCache.lastAccessedTime = st.st_atim;
		objectDiskCache.filepath = _objectFolder + dent->d_name;

		{
			lock_guard<mutex> lk(diskCacheMutex);
			_objectDiskCacheMap.set(objectId, objectDiskCache);
			_objectCacheQueue.push_back(objectId);
		}

		_freeObjectSpace -= st.st_size;
		_currentObjectUsage += st.st_size;

		cout << "ID: " << objectId << "\tLength: " << objectDiskCache.length
				<< "\t Modified: " << objectDiskCache.lastAccessedTime.tv_sec
				<< endl;

	}
	closedir(srcdir);

	cout << "=======================" << endl;

	cout << "Object Cache Usage: " << formatSize(_currentObjectUsage) << "/"
			<< formatSize(_maxObjectCache) << endl;

	//
	// initialize segments
	//

	srcdir = opendir(_segmentFolder.c_str());
	if (srcdir == NULL) {
		perror("opendir");
		exit(-1);
	}

	while ((dent = readdir(srcdir)) != NULL) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			continue;

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
			perror(dent->d_name);
			continue;
		}

		// save file info
		_freeSegmentSpace -= st.st_size;
		_currentSegmentUsage += st.st_size;
	}
	closedir(srcdir);

	cout << "Segment Storage Usage: " << formatSize(_currentSegmentUsage) << "/"
			<< formatSize(_maxSegmentCapacity) << endl;

}

uint64_t StorageModule::getFilesize(string filepath) {

	ifstream in(filepath, ifstream::in | ifstream::binary);

	if (!in) {
		debug("ERROR: Cannot open file: %s\n", filepath.c_str());
		perror("ifstream");
		exit(-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	in.close();

	return filesize;

}

void StorageModule::createObjectTransferCache(uint64_t objectId, uint32_t length) {

	// create cache
	struct ObjectTransferCache objectTransferCache;
	objectTransferCache.length = length;
	objectTransferCache.buf = MemoryPool::getInstance().poolMalloc(length);

	// save cache to map
	{
		lock_guard<mutex> lk(transferCacheMutex);
		_objectTransferCache[objectId] = objectTransferCache;
	}

	debug("Object created ID = %" PRIu64 " Length = %" PRIu32 "\n",
			objectId, length);
}

void StorageModule::createObjectDiskCache(uint64_t objectId, uint32_t length) {
	// create object
	createAndOpenObjectFile(objectId, length);

	// write info
	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);
}

void StorageModule::createSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {
	createAndOpenSegment(objectId, segmentId, length);

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);

	debug(
			"Segment created ObjID = %" PRIu64 " SegmentID = %" PRIu32 " Length = %" PRIu32 " Path = %s\n",
			objectId, segmentId, length, filepath.c_str());
}

bool StorageModule::isObjectCached(uint64_t objectId) {

#ifndef USE_OBJECT_CACHE
	return false;
#endif

	lock_guard<mutex> lk(diskCacheMutex);
	if (_objectDiskCacheMap.count(objectId)) {

		_objectDiskCacheMap.get(objectId).lastAccessedTime = {time(NULL), 0};

		_objectCacheQueue.remove(objectId);
		_objectCacheQueue.push_back(objectId);

		return true;
	}
	return false;
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

	closeFile(segmentPath);
	return segmentData;
}

uint32_t StorageModule::writeObjectTransferCache(uint64_t objectId, char* buf,
		uint64_t offsetInObject, uint32_t length) {

	char* recvCache;

	{
		lock_guard<mutex> lk(transferCacheMutex);
		if (!_objectTransferCache.count(objectId)) {
			debug("%s\n", "cannot find cache for object");
			cout << "writeObjectCache Object Cache Not Found " << objectId
					<< endl;
			exit(-1);
		}
		recvCache = _objectTransferCache[objectId].buf;
	}

	memcpy(recvCache + offsetInObject, buf, length);

	return length;
}

uint32_t StorageModule::writeObjectDiskCache(uint64_t objectId, char* buf,
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

	updateSegmentFreespace(length);
//	closeFile(filepath);

	return byteWritten;
}

FILE* StorageModule::createAndOpenObjectFile(uint64_t objectId,
		uint32_t length) {

	string filepath = generateObjectPath(objectId, _objectFolder);
	return createFile(filepath);
}

FILE* StorageModule::createAndOpenSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);

	debug("Object ID = %" PRIu64 " Segment ID = %" PRIu32 " created\n",
			objectId, segmentId);

	return createFile(filepath);
}

void StorageModule::closeObjectTransferCache(uint64_t objectId) {
	// close cache
	struct ObjectTransferCache objectCache = getObjectTransferCache(objectId);
	MemoryPool::getInstance().poolFree(objectCache.buf);

	{
		lock_guard<mutex> lk(transferCacheMutex);
		_objectTransferCache.erase(objectId);
	}

	debug("Object Cache ID = %" PRIu64 " closed\n", objectId);
}

void StorageModule::closeObjectDiskCache(uint64_t objectId) {
	// close file
	string filepath = generateObjectPath(objectId, _objectFolder);
	closeFile(filepath);

	debug("Object ID = %" PRIu64 " closed\n", objectId);
}

void StorageModule::closeSegment(uint64_t objectId, uint32_t segmentId) {
	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	closeFile(filepath);

	debug("Object ID = %" PRIu64 " Segment ID = %" PRIu32 " closed\n",
			objectId, segmentId);
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
	objectInfo.objectSize = getFilesize(objectInfo.objectPath);

	return objectInfo;
}

uint32_t StorageModule::readFile(string filepath, char* buf, uint64_t offset,
		uint32_t length) {

	debug("Read File :%s\n", filepath.c_str());

	// lock file access function
	lock_guard<mutex> lk(fileMutex);

	FILE* file = openFile(filepath);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot read");
		perror("open");
		exit(-1);
	}

	// Read file contents into buffer
	uint32_t byteRead = pread(fileno(file), buf, length, offset);

	if (byteRead != length) {
		debug("ERROR: Length = %" PRIu32 ", byteRead = %" PRIu32 "\n",
				length, byteRead);
		perror("pread()");
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
		perror("open");
		exit(-1);
	}

	// Write file contents from buffer
	uint32_t byteWritten = pwrite(fileno(file), buf, length, offset);

	if (byteWritten != length) {
		debug("ERROR: Length = %d, byteWritten = %d\n", length, byteWritten);
		perror("pwrite()");
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

	if (filePtr == NULL) {
		debug("%s\n", "Unable to create file!");
		return NULL;
	}

	// set buffer to zero to avoid memory leak
	setvbuf(filePtr, NULL, _IONBF, 0);

	// add file pointer to map
	_openedFile->insert(filepath, filePtr);

	/*
	openedFileMutex.lock();
	_openedFile[filepath] = filePtr;
	openedFileMutex.unlock();
	*/

	return filePtr;
}

/**
 * Open an existing file, return pointer directly if file is already open
 */

FILE* StorageModule::openFile(string filepath) {

	FILE* filePtr = NULL;
	try {
		filePtr = _openedFile->get (filepath);
	} catch (out_of_range& oor) { // file pointer not found in cache
		filePtr = fopen(filepath.c_str(), "rb+");

		if (filePtr == NULL) {
			debug("Unable to open file at %s\n", filepath.c_str());
			perror("fopen()");
			return NULL;
		}

		// set buffer to zero to avoid memory leak
		setvbuf(filePtr, NULL, _IONBF, 0);

		// add file pointer to map
		_openedFile->insert(filepath, filePtr);
	}

	return filePtr;
}

/**
 * Close file, remove from map
 */

void StorageModule::closeFile(string filepath) {
	/*
	FILE* filePtr = openFile(filepath);

	openedFileMutex.lock();
	_openedFile.erase(filepath);
	fclose(filePtr);
	openedFileMutex.unlock();
	*/
}

struct ObjectTransferCache StorageModule::getObjectTransferCache(uint64_t objectId) {
	lock_guard<mutex> lk(transferCacheMutex);
	if (!_objectTransferCache.count(objectId)) {
		debug("%s\n", "object cache not found");
		cout << "GetObjectCache Object Cache Not Found " << objectId << endl;
		exit(-1);
	}
	return _objectTransferCache[objectId];
}

void StorageModule::setMaxSegmentCapacity(uint32_t max_segment) {
	_maxSegmentCapacity = max_segment;
}

void StorageModule::setMaxObjectCache(uint32_t max_object) {
	_maxObjectCache = max_object;
}

uint32_t StorageModule::getMaxSegmentCapacity() {
	return _maxSegmentCapacity;
}

uint32_t StorageModule::getMaxObjectCache() {
	return _maxObjectCache;
}

bool StorageModule::verifySegmentSpace(uint32_t size) {
	return size <= _freeSegmentSpace ? true : false;
}

bool StorageModule::verifyObjectSpace(uint32_t size) {
	return size <= _freeObjectSpace ? true : false;
}

void StorageModule::updateSegmentFreespace(uint32_t new_segment_size) {
	uint32_t update_space = new_segment_size;
	if (verifySegmentSpace(update_space)) {
		_currentSegmentUsage += update_space;
		_freeSegmentSpace -= update_space;
	} else {
		perror("segment free space not enough.\n");
	}

}

uint32_t StorageModule::getCurrentSegmentCapacity() {
	return _currentSegmentUsage;
}

uint32_t StorageModule::getCurrentObjectCache() {
	return _currentObjectUsage;
}

uint32_t StorageModule::getFreeSegmentSpace() {
	return _freeSegmentSpace;
}

uint32_t StorageModule::getFreeObjectSpace() {
	return _freeObjectSpace;
}

int32_t StorageModule::spareObjectSpace(uint32_t newObjectSize) {

	if (_maxObjectCache < newObjectSize) {
		return -1; // error: object size larger than cache
	}

	while (_freeObjectSpace < newObjectSize) {
		uint64_t objectId = 0;
		struct ObjectDiskCache objectCache;

		{
			lock_guard<mutex> lk(diskCacheMutex);
			objectId = _objectCacheQueue.front();
			objectCache = _objectDiskCacheMap.get(objectId);
		}

		remove(objectCache.filepath.c_str());

		// update size
		_freeObjectSpace += objectCache.length;
		_currentObjectUsage -= objectCache.length;

		// remove from queue and map
		{
			lock_guard<mutex> lk(diskCacheMutex);
			_objectCacheQueue.remove(objectId);
			_objectDiskCacheMap.erase(objectId);
		}
	}

	return 0;

}

void StorageModule::putObjectToDiskCache(uint64_t objectId,
		ObjectTransferCache objectCache) {

	debug("Before saving object ID = %" PRIu64 ", cache = %s\n",
			objectId, formatSize(_freeObjectSpace).c_str());

	uint32_t objectSize = objectCache.length;

	{

		lock_guard<mutex> lk(diskSpaceMutex);

		if (!verifyObjectSpace(objectSize)) {
			// clear cache if space is not available
			//updateObjectFreespace(spareObjectSpace(objectSize));
			if (spareObjectSpace(objectSize) == -1) {
				debug(
						"Not enough space to cache object! Object Size = %" PRIu32 "\n",
						objectSize);
				return;
			}
		}

	}

	debug("Spare space for saving object ID = %" PRIu64 ", cache = %s\n",
			objectId, formatSize(_freeObjectSpace).c_str());

	// write cache to disk
	createAndOpenObjectFile(objectId, objectCache.length);

#ifdef USE_OBJECT_CACHE
	uint64_t byteWritten = writeObjectDiskCache(objectId, objectCache.buf, 0,
			objectCache.length);
#else
	uint64_t byteWritten = objectCache.length;
#endif

	if (byteWritten != objectCache.length) {
		perror("Cannot saveObjectToDisk");
		exit(-1);
	}
	closeObjectDiskCache(objectId);

	_currentObjectUsage += objectSize;
	_freeObjectSpace -= objectSize;

	// save cache to map
	struct ObjectDiskCache objectDiskCache;
	objectDiskCache.filepath = generateObjectPath(objectId, _objectFolder);
	objectDiskCache.length = objectCache.length;
	objectDiskCache.lastAccessedTime = {time(NULL), 0}; // set to current time

	{
		lock_guard<mutex> lk(diskCacheMutex);
		_objectDiskCacheMap.set(objectId, objectDiskCache);
		_objectCacheQueue.push_back(objectId);
	}

	debug("After saving object ID = %" PRIu64 ", cache = %s\n",
			objectId, formatSize(_freeObjectSpace).c_str());

}

struct ObjectData StorageModule::getObjectFromDiskCache(uint64_t objectId) {
	struct ObjectData objectData;
	struct ObjectDiskCache objectDiskCache;
	{
		lock_guard<mutex> lk(diskCacheMutex);
		objectDiskCache = _objectDiskCacheMap.get(objectId);
	}
	objectData = readObject(objectId, 0, objectDiskCache.length);
	return objectData;
}

void StorageModule::clearObjectDiskCache() {

	for (auto object : _objectCacheQueue) {
		remove(string(_objectFolder + to_string(object)).c_str());
	}

	{
		lock_guard<mutex> lk(diskCacheMutex);
		_objectCacheQueue.clear();
		_objectDiskCacheMap.clear();
	}

	_freeObjectSpace = _maxObjectCache;
	_currentObjectUsage = 0;
}
