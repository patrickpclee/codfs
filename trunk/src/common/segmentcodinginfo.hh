#ifndef SEGMENTCODINGINFO_HH_
#define SEGMENTCODINGINFO_HH_

#include "enums.hh"

struct SegmentCodingInfo {
    uint64_t segmentId;
    uint32_t segmentSize;
    uint32_t blockSize;
    CodingScheme codingScheme;
    std::string codingSetting;
};

#endif
