#ifndef __OSDSTAT_HH__
#define __OSDSTAT_HH__

struct OsdStat {
	uint32_t osdId;
	uint32_t osdIP;
	uint16_t osdPort;
	uint32_t osdCapacity;
	uint32_t osdLoading;
	uint32_t osdHealth;	// UP DOWN OUT
};


#endif
