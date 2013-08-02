#ifndef __SEGMENTDATA_HH__
#define __SEGMENTDATA_HH__

#include <stdint.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include "../common/define.hh"
#include "../common/enums.hh"

using namespace std;

struct SegmentInfo {
	uint64_t segmentId;
	uint32_t segLength;
	string segmentPath;
    vector<offset_length_t> offlenVector; 

    void packOffsets() {
        vector<offset_length_t> packedVector;
        packedVector.clear();
        sort(offlenVector.begin(), offlenVector.end());
        uint32_t curOffsetStart = offlenVector[0].first;
        uint32_t curOffsetEnd = offlenVector[0].first + offlenVector[0].second;
        for (uint32_t i = 1; i < offlenVector.size(); i++) {
            if (offlenVector[i].first > curOffsetEnd) {
                // Seal current offset,length
                packedVector.push_back(make_pair(curOffsetStart, curOffsetEnd-curOffsetStart));
                curOffsetStart = offlenVector[i].first;
                curOffsetEnd = offlenVector[i].first + offlenVector[i].second;
            } else {
                curOffsetEnd = std::max(curOffsetEnd, offlenVector[i].first + offlenVector[i].second);
            }
        }
        // Seal last offset,length
        packedVector.push_back(make_pair(curOffsetStart, curOffsetEnd-curOffsetStart));

        // update the offlenVector by swapping (constant time)
        offlenVector.swap(packedVector);
    };
};

struct SegmentData {
	struct SegmentInfo info;
	char* buf;
    uint32_t bufLength;
    FileType fileType;

    void packBuf() {
    	if (info.offlenVector.size() == 0) {
    		bufLength = info.segLength;
    		return;
    	}
        info.packOffsets();
        bufLength = 0;
        for (uint32_t i = 0; i < info.offlenVector.size(); i++) {
            memmove(buf+bufLength, buf+info.offlenVector[i].first, info.offlenVector[i].second);
            bufLength += info.offlenVector[i].second;
        }
    };
};

#endif
