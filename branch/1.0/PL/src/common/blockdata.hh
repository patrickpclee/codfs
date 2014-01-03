#ifndef __BLOCKDATA_HH__
#define __BLOCKDATA_HH__

#include <string>
#include <stdint.h>
#include "../common/blocklocation.hh"
#include "../common/enums.hh"

using namespace std;

struct BlockInfo {
	uint64_t segmentId;
	uint32_t blockId;
	uint32_t blockSize;
    vector<BlockLocation> parityVector; // pair of <osdid, blockid>
    vector<offset_length_t> offlenVector;
    
    // only for update
    CodingScheme codingScheme;
    string codingSetting;
    uint64_t segmentSize;

    BlockInfo() {
        segmentId = 0;
        blockId = 0;
        blockSize = 0;
        codingScheme = DEFAULT_CODING;
        codingSetting = "";
        segmentSize = 0;
    }

};

struct BlockData {
	struct BlockInfo info;
	char* buf;
};

#endif
