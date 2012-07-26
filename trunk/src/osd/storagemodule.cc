/*
 * storagemodule.cc
 */

#include <sstream>
#include "storagemodule.hh"
#include "../common/debug.hh"

// global variable defined in each component
extern ConfigLayer* configLayer;

StorageModule::StorageModule() {
	_objectFolder = configLayer->getConfigString("Storage>ObjectLocation");
	_segmentFolder = configLayer->getConfigString("Storage>SegmentLocation");
	_capacity = 0;
	_freespace = 0;
}

StorageModule::~StorageModule() {

}

void StorageModule::createObject(uint64_t objectId, uint32_t length) {

}

void StorageModule::createSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {

}

bool StorageModule::isObjectExist(uint64_t objectId) {

	ObjectInfo objectInfo = readObjectInfo(objectId);
	if (objectInfo.objectSize == 0) {
		return false;
	}

	return true;
}

struct ObjectData StorageModule::readObject(uint64_t objectId,
		uint64_t offsetInObject, uint32_t length) {

	struct ObjectData objectData;
	objectData.info = readObjectInfo(objectId);

	// obtain enough memory for the object
	char* buf = objectData.buf;

	// check num of bytes to read
	// if length = 0, read whole object
	uint32_t byteToRead;
	if (length == 0) {
		byteToRead = objectData.info.objectSize;
	} else {
		byteToRead = length;
	}

	// TODO: check maximum malloc size
	// TODO: when free?
	buf = MemoryPool::getInstance().poolMalloc(byteToRead);

	readFile(objectData.info.objectPath, buf,
			offsetInObject, byteToRead);

	return objectData;

}

struct SegmentData StorageModule::readSegment(uint64_t objectId,
		uint32_t segmentId, uint64_t offsetInSegment, uint32_t length) {

	struct SegmentData segmentData;
	segmentData.info = readSegmentInfo(objectId, segmentId);

	// obtain enough memory for the segment
	char* buf = segmentData.buf;

	// check num of bytes to read
	// if length = 0, read whole segment
	uint32_t byteToRead;
	if (length == 0) {
		byteToRead = segmentData.info.segmentSize;
	} else {
		byteToRead = length;
	}

	// TODO: check maximum malloc size
	// TODO: when free?
	buf = MemoryPool::getInstance().poolMalloc(byteToRead);

	readFile(segmentData.info.segmentPath, buf,
			offsetInSegment, byteToRead);

	return segmentData;
}

uint32_t StorageModule::writeObject(uint64_t objectId, char* buf,
		uint64_t offsetInObject, uint32_t length) {

	uint32_t byteWritten;

	string filepath = generateObjectPath(objectId, _objectFolder);
	byteWritten = writeFile(filepath, buf, offsetInObject, length);

	return byteWritten;
}

uint32_t StorageModule::writeSegment(uint64_t objectId, uint32_t segmentId,
		char* buf, uint64_t offsetInSegment, uint32_t length) {

	uint32_t byteWritten;

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	byteWritten = writeFile(filepath, buf, offsetInSegment, length);

	return byteWritten;
}

FILE* StorageModule::createAndOpenObject(uint64_t objectId, uint32_t length) {

	string filepath = generateObjectPath(objectId, _objectFolder);
	writeObjectInfo(objectId, length, filepath);

	return createFile(filepath);
}

FILE* StorageModule::createAndOpenSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {

	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	writeSegmentInfo(objectId, segmentId, length, filepath);

	return createFile(filepath);
}

void StorageModule::closeObject(uint64_t objectId) {
	string filepath = generateObjectPath(objectId, _objectFolder);
	closeFile(filepath);
}

void StorageModule::closeSegment(uint64_t objectId, uint32_t segmentId) {
	string filepath = generateSegmentPath(objectId, segmentId, _segmentFolder);
	closeFile(filepath);
}

uint32_t StorageModule::getCapacity() {
	return _capacity;
}

uint32_t StorageModule::getFreespace() {
	return _freespace;
}

//
// PRIVATE METHODS
//

void StorageModule::writeObjectInfo(uint64_t objectId, uint32_t objectSize,
		string filepath) {

}

struct ObjectInfo StorageModule::readObjectInfo(uint64_t objectId) {
	struct ObjectInfo objectInfo;
	return objectInfo;
}

void StorageModule::writeSegmentInfo(uint64_t objectId, uint32_t segmentId,
		uint32_t segmentSize, string filepath) {

}

struct SegmentInfo StorageModule::readSegmentInfo(uint64_t objectId,
		uint32_t segmentId) {
	struct SegmentInfo segmentInfo;
	return segmentInfo;
}

uint32_t StorageModule::readFile(string filepath, char* buf, uint64_t offset,
		uint32_t length) {

	FILE* file = openFile(filepath);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot read");
		return -1;
	}

	// Read file contents into buffer
	fseek(file, offset, SEEK_SET);
	uint32_t byteRead = fread(buf, length, 1, file);

	if (byteRead != length) {
		debug("ERROR: Length = %d, byteRead = %d\n", length, byteRead);
	}

	return byteRead;
}

uint32_t StorageModule::writeFile(string filepath, char* buf, uint64_t offset,
		uint32_t length) {

	FILE* file = openFile(filepath);

	if (file == NULL) { // cannot open file
		debug("%s\n", "Cannot write");
		return -1;
	}

	// TODO: lock here

	// Write file contents from buffer
	fseek(file, offset, SEEK_SET);

	uint32_t byteWritten = fwrite(buf, length, 1, file);

	if (byteWritten != length) {
		debug("ERROR: Length = %d, byteRead = %d\n", length, byteWritten);
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

	// add file pointer to map
	_openedFile[filepath] = filePtr;
	return filePtr;
}

/**
 * Open an existing file, return pointer directly if file is already open
 */

FILE* StorageModule::openFile(string filepath) {

	// find file in map
	if (_openedFile.count(filepath)) {
		return _openedFile[filepath];
	}

	FILE* filePtr;
	filePtr = fopen(filepath.c_str(), "rb+");

	if (filePtr == NULL) {
		debug("%s\n", "Unable to open file!");
		return NULL;
	}

	// add file pointer to map
	_openedFile[filepath] = filePtr;
	return filePtr;
}

/**
 * Close file, remove from map
 */

void StorageModule::closeFile(string filepath) {

	FILE* filePtr = openFile(filepath);
	_openedFile.erase(filepath);
	fclose(filePtr);
}
