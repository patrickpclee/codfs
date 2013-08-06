#ifndef __BLOCKDATA_HH__
#define __BLOCKDATA_HH__

#include <string>
#include <stdint.h>
#include "../common/blocklocation.hh"

using namespace std;

struct BlockInfo {
	uint64_t segmentId;
	uint32_t blockId;
	uint32_t blockSize;
    vector<BlockLocation> parityVector; // pair of <osdid, blockid>
    vector<offset_length_t> offlenVector;

};

struct BlockData {
	struct BlockInfo info;
	char* buf;
};

#endif
