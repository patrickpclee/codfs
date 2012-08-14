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

ClientStorageModule::ClientStorageModule() {
	// read config value
	_objectSize = configLayer->getConfigLong("Storage>ObjectSize") * 1024;
	debug("Config Object Size = %" PRIu64 " Bytes\n", _objectSize);
}

uint64_t ClientStorageModule::getFilesize (string filepath) {

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

	const uint64_t filesize = getFilesize (filepath);

	if (filesize == 0) {
		return 0;
	}

	uint32_t objectCount = (uint32_t) ((filesize - 1) / _objectSize + 1);

	return objectCount;
}

struct ObjectData ClientStorageModule::readObjectFromFile(string filepath,
		uint32_t objectIndex) {

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
