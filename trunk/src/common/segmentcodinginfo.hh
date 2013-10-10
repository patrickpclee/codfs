#ifndef SEGMENTCODINGINFO_HH_
#define SEGMENTCODINGINFO_HH_

struct SegmentCodingInfo {
    uint64_t segmentId;
    uint32_t segmentSize;
    CodingScheme codingScheme;
    std::string codingSetting;
};

#endif
