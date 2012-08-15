#ifndef __OSDSTAT_HH__
#define __OSDSTAT_HH__

#include <thread>
using namespace std;

extern mutex osdStatMapMutex;

enum OsdHealthStat{
	ONLINE, OFFLINE
};

struct OsdStat {
	OsdStat() { }
	OsdStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
		uint32_t load, enum OsdHealthStat health): osdId(id), osdSockfd(sockfd),
		osdCapacity(cap), osdLoading(load),	osdHealth(health) { } 
	OsdStat(uint32_t id, uint32_t sockfd, uint32_t cap, 
		uint32_t load,  enum OsdHealthStat health, uint32_t ip, uint16_t port): 
		osdId(id), osdSockfd(sockfd), osdCapacity(cap), osdLoading(load),
		osdHealth(health), osdIp(ip), osdPort(port) { } 
		
	void out() {
		printf("OSD[id=%d],cap=%d load=%d\n",osdId, osdCapacity, osdLoading);
	}
	uint32_t osdId;
	uint32_t osdSockfd;
	uint32_t osdCapacity;
	uint32_t osdLoading;
	enum OsdHealthStat osdHealth;	// UP DOWN OUT
	uint32_t osdIp;
	uint32_t osdPort;
};


#endif
