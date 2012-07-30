#include <fstream>
#include <math.h>
#include "stdlib.h"
#include "../common/debug.hh"
#include "../common/memorypool.hh"
#include "client_storagemodule.hh"
#include "../config/config.hh"
using namespace std;

/// Config Layer
extern ConfigLayer* configLayer;

ClientStorageModule::ClientStorageModule() {
	// read config value
	_objectSize = configLayer->getConfigLong("Storage>ObjectSize") * 1024;
	debug ("Config Object Size = %lu Bytes\n", _objectSize);
}

uint32_t ClientStorageModule::getObjectCount(string filepath) {
	ifstream in(filepath, ifstream::in | ifstream::binary);

	if (!in) {
		debug("%s\n", "ERROR: Cannot open file");
		exit (-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	in.close();

	if (filesize == 0) {
		return 0;
	}

	uint32_t objectCount = (uint32_t) ((filesize - 1) / _objectSize + 1);

	return objectCount;
}

struct ObjectData ClientStorageModule::readObjectFromFile(string filepath,
		uint32_t objectIndex) {

	struct ObjectData objectData;
	uint32_t byteToRead = 0;

	// index starts from 0
	const uint64_t offset = objectIndex * _objectSize;

	ifstream in(filepath, ifstream::in | ifstream::binary);

	if (!in) {
		debug("%s\n", "ERROR: Cannot open file");
		exit (-1);
	}

	// check filesize
	in.seekg(0, std::ifstream::end);
	uint64_t filesize = in.tellg();

	if (offset >= filesize) {
		debug("%s\n", "ERROR: offset exceeds filesize");
		exit (-1);
	}

	if (filesize - offset > _objectSize) {
		byteToRead = _objectSize;
	} else {
		byteToRead = filesize - offset;
	}

	// read into objectdata buffer
	objectData.buf = MemoryPool::getInstance().poolMalloc(_objectSize); // TODO: when free object?
	in.seekg(offset, std::ifstream::end);
	in.read(objectData.buf, byteToRead);
	in.close();

	// fill in information about object
	objectData.info.objectSize = byteToRead;

	return objectData;

}
