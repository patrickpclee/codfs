#ifndef RESERVESPACEINFO_HH_
#define RESERVESPACEINFO_HH_

struct ReserveSpaceInfo {
    uint32_t remainingReserveSpace;
    uint32_t currentOffset;
    uint32_t blockSize;
};

#endif
