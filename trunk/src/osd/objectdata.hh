#ifndef __OBJECTDATA_HH__
#define __OBJECTDATA_HH__

#include <string>

using namespace std;

struct ObjectData {
	uint64_t objectId;
	uint32_t offsetInFile;
	uint32_t length;
	string ObjectPath;
	char* buf;
};

#endif
