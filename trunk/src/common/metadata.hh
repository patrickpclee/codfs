#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <stdint.h>
#include <string>
#include <vector>

#include "enums.hh"

using namespace std;

// use in client
struct SegmentTransferClientInfo {
	uint64_t _id;
	uint32_t _size;
	uint32_t _chunkCount;
};

// use in OSD
struct SegmentTransferOsdInfo {
	uint64_t _id;
	uint32_t _size;
	vector <uint32_t> _osdList;
	CodingScheme _codingScheme;
	string _codingSetting;
};

struct FileMetaData {
	string _path;
	uint32_t _id;
	uint64_t _size;
	vector<uint64_t> _segmentList;
	vector<uint32_t> _primaryList;

	//uint64_t* _segmentList[];
	FileType _fileType;	
};

struct SegmentMetaData {
	uint64_t _id;
	uint32_t _size;
	//uint32_t _blockCount;
	//uint32_t* _blockList;
	//uint32_t _offsetInFile;
	vector<uint32_t> _nodeList;
	uint32_t _primary;
	CodingScheme _codingScheme;
	string _codingSetting;
};

struct BlockMetaData {
	uint32_t _id;
	uint32_t _offsetInSegment;
};

#endif
