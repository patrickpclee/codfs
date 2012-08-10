#ifndef __OSDSTAT_HH__
#define __OSDSTAT_HH__

#include <thread>
using namespace std;

extern mutex osdStatMapMutex;

struct OsdStat {
	OsdStat() { }
	OsdStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
		uint32_t load, uint32_t health): osdId(id), osdSockfd(sockfd),
		osdCapacity(cap), osdLoading(load),	osdHealth(health) { } 
	uint32_t osdId;
	uint32_t osdSockfd;
	uint32_t osdCapacity;
	uint32_t osdLoading;
	uint32_t osdHealth;	// UP DOWN OUT
};


#endif
