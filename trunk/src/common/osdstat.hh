#ifndef __OSDSTAT_HH__
#define __OSDSTAT_HH__

#include <thread>
#include <mutex>
using namespace std;

extern mutex osdStatMapMutex;

enum OsdHealthStat{
	ONLINE, OFFLINE, RECOVERING
};

struct OsdStat {
	OsdStat() { }
	OsdStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
			uint32_t load, enum OsdHealthStat health, uint32_t ts): osdId(id), osdSockfd(sockfd),
	osdCapacity(cap), osdLoading(load),	osdHealth(health), timestamp(ts) { } 
	OsdStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
			uint32_t load,  enum OsdHealthStat health, uint32_t ip, uint16_t port,
			uint32_t ts): 
		osdId(id), osdSockfd(sockfd), osdCapacity(cap), osdLoading(load),
		osdHealth(health), osdIp(ip), osdPort(port), timestamp(ts) { } 

	void out() {
		printf("OSD[id=%d,ip=%d,port=%d,sockfd=%d],cap=%d load=%d health =%d\n",osdId, osdIp, osdPort, osdSockfd, osdCapacity, osdLoading, osdHealth);
	}
	uint32_t osdId;
	uint32_t osdSockfd;
	uint32_t osdCapacity;
	uint32_t osdLoading;
	enum OsdHealthStat osdHealth;	// UP DOWN OUT
	uint32_t osdIp;
	uint32_t osdPort;
	uint32_t timestamp;
};

struct OsdLBStat {
	OsdLBStat() {}
	OsdLBStat(uint64_t primaryCount, uint64_t diskCount):
		primaryCount(primaryCount), diskCount(diskCount) {}
	uint64_t primaryCount;
	uint64_t diskCount;
};


#endif
