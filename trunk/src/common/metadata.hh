#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <stdint.h>
#include <string>
#include <vector>

#include "enums.hh"

using namespace std;

struct FileMetaData {
	string _path;
	uint32_t _id;
	uint64_t _size;
	vector<uint64_t> objectList;
	vector<uint32_t> primaryList;

	unsigned char* _checksum;
	//uint64_t* _objectList[];
	FileType _fileType;	
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
