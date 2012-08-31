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

// global variable defined in each component
extern ConfigLayer* configLayer;

mutex fileMutex;
mutex openedFileMutex;
mutex cacheMutex;

StorageModule::StorageModule() {
	_openedFile = {};
	_objectCache = {};
	_objectFolder = configLayer->getConfigString("Storage>ObjectCacheLocation");
	_segmentFolder = configLayer->getConfigString("Storage>SegmentLocation");

	// Unit in XML: GB
	// Unit in StorageModule: Bytes
	_maxObjectCache = configLayer->getConfigInt("Storage>ObjectCacheCapacity")
	* 1073741824ULL;
	_maxSegmentCapacity = configLayer->getConfigInt("Storage>SegmentCapacity")
	* 1073741824ULL;

	cout << "=== STORAGE ===" << endl;
	cout << "Object Cache Location = " << _objectFolder << " Size = "
	<< formatSize(_maxObjectCache) << endl;
	cout << "Segment Storage Location = " << _segmentFolder << " Size = "
	<< formatSize(_maxSegmentCapacity) << endl;
	cout << "===============" << endl;

	/*
	 _freeSegmentSpace = _maxSegmentCapacity;
	 _freeObjectSpace = _maxObjectCache;
	 _currentSegment = 0;
	 _currentObject = 0;
	 */

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
		objectDiskCache.lastModifiedTime = st.st_mtim;
		objectDiskCache.filepath = _objectFolder + dent->d_name;
		_objectDiskCacheMap[objectId] = objectDiskCache;

		_freeObjectSpace -= st.st_size;
		_currentObjectUsage += st.st_size;

		cout << "ID: " << objectId << "\tLength: "
				<< objectDiskCache.length << "\t Modified: "
				<< objectDiskCache.lastModifiedTime.tv_sec << endl;

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
		exit(-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	in.close();

	return filesize;

}

void StorageModule::createObjectCache(uint64_t objectId, uint32_t length) {

	// create cache
	struct ObjectTransferCache objectCache;
	objectCache.length = length;
	objectCache.buf = MemoryPool::getInstance().poolMalloc(length);

	// save cache to map
	{
		lock_guard<mutex> lk(cacheMutex);
		_objectCache[objectId] = objectCache;
	}

	debug("Object created ID = %" PRIu64 " Length = %" PRIu32 "\n",
			objectId, length);
}

void StorageModule::createObjectFile(uint64_t objectId, uint32_t length) {
	// create object
	createAndOpenObjectFile(objectId, length);

	// write info
	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);

	//TODO save object to the QUEUE
	updateObjectFreespace(length);
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

uint32_t StorageModule::writeObjectFile(uint64_t objectId, char* buf,
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
	writeSegmentInfo(objectId, segmentId, length, filepath);

	debug("Object ID = %" PRIu64 " Segment ID = %" PRIu32 " created\n",
			objectId, segmentId);

	return createFile(filepath);
}

void StorageModule::closeObjectCache(uint64_t objectId) {
	// close cache
	struct ObjectTransferCache objectCache = getObjectCache(objectId);
	MemoryPool::getInstance().poolFree(objectCache.buf);

	{
		lock_guard<mutex> lk(cacheMutex);
		_objectCache.erase(objectId);
	}

	debug("Object Cache ID = %" PRIu64 " closed\n", objectId);
}

void StorageModule::closeObjectFile(uint64_t objectId) {
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

struct ObjectTransferCache StorageModule::getObjectCache(uint64_t objectId) {
	lock_guard<mutex> lk(cacheMutex);
	if (!_objectCache.count(objectId)) {
		debug("%s\n", "object cache not found");
		exit(-1);
	}
	return _objectCache[objectId];
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

void StorageModule::updateObjectFreespace(uint32_t new_object_size) {
	uint32_t update_space = new_object_size;
	if (verifyObjectSpace(update_space)) {
		_currentObjectUsage += update_space;
		_freeObjectSpace -= update_space;
	} else {
		perror("object free space not enough.\n");
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

int32_t StorageModule::spareObjectSpace(uint32_t new_object_size){
	//TODO delete old objects and make room for new one.
	return 0;
}

void StorageModule::saveObjectToDisk(ObjectTransferCache objectCache) {
	//TODO write object to disk.


	uint32_t update_size = objectCache.length;
	if(verifyObjectSpace(update_size)){
		updateObjectFreespace(update_size);
	}else{
		updateObjectFreespace(spareObjectSpace(update_size));
	}
}
