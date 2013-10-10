#ifndef SEGMENTCODINGINFO_HH_
#define SEGMENTCODINGINFO_HH_

#include "enums.hh"

struct SegmentCodingInfo {
    uint64_t segmentId;
    uint32_t segmentSize;
    CodingScheme codingScheme;
    std::string codingSetting;
};

#endif
