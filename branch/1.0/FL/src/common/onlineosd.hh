#ifndef ONLINEOSD_HH_
#define ONLINEOSD_HH_

struct OnlineOsd {
	OnlineOsd() { }
	OnlineOsd(uint32_t id, uint32_t ip, uint32_t port):
		osdId(id), osdIp(ip), osdPort(port) { }
	uint32_t osdId;
	uint32_t osdIp;
	uint32_t osdPort;
};

#endif /* ONLINEOSD_HH_ */
