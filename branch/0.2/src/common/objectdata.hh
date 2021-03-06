#ifndef __OBJECTDATA_HH__
#define __OBJECTDATA_HH__

#include <stdint.h>
#include <string>

using namespace std;

struct ObjectInfo {
	uint64_t objectId;
	uint32_t objectSize;
	string objectPath;
};

struct ObjectData {
	struct ObjectInfo info;
	char* buf;
};

#endif
