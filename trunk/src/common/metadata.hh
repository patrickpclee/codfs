#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <stdint.h>
#include <string>
using namespace std;

struct FileMetaData {
	string _path;
	uint32_t _id;
	unsigned char* _checksum;
	uint64_t* _objectList[];
};

struct ObjectMetaData {
	uint64_t _id;
	uint32_t _segmentCount;
	uint32_t* _segmentList;
	uint32_t _offsetInFile;
	unsigned char* _checksum;
};

struct SegmentMetaData {
	uint32_t _id;
	uint32_t _offsetInObject;
	unsigned char* _checksum;
};

#endif
