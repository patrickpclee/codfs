#ifndef __STATMODULE_HH__
#define __STATMODULE_HH__

#include <stdint.h>
#include <map>
#include "../common/osdstat.hh"
#include "../communicator/communicator.hh"
#include "../protocol/status/osdstatupdaterequestmsg.hh"

using namespace std;

class StatModule {

public:
	/**
	 * Constructor for statmodule
	 * @param mapRef Reference of the map which store all the osd status
	 */
	StatModule(map<uint32_t, struct OsdStat>& mapRef);

	/**
	 * Periodically update the osd stat map by communicate each osd
	 * @param communicator Monitor communicator 
	 */
	void updateOsdStatMap (Communicator* communicator);

	/**  
	 * Remove an osd status entry by its osdId 
	 * @param osdId OSD Id
	 */
	void removeStatById (uint32_t osdId);

	/**
	 * Set an osd status entry, if not in the map, create it, else update the result
	 * @param osdId OSD ID
	 * @param sockfd Sockfd between the monitor and the osd
	 * @param capacity Free space of the OSD
	 * @param loading Current CPU loading of the OSD
	 * @param health Health status of the OSD
	 */
	void setStatById (uint32_t osdId, uint32_t sockfd, uint32_t capacity,
		 uint32_t loading, enum OsdHealthStat health);

	/**
	 * Set an osd status entry, if not in the map, create it, else update the result
	 * @param osdId OSD ID
	 * @param sockfd Sockfd between the monitor and the osd
	 * @param capacity Free space of the OSD
	 * @param loading Current CPU loading of the OSD
	 * @param health Health status of the OSD
	 * @param ip IP of the OSD for other components' connection
	 * @param port Port of the OSD for other components' connetion
	 */
	void setStatById (uint32_t osdId, uint32_t sockfd, uint32_t capacity,
		 uint32_t loading, enum OsdHealthStat health, uint32_t ip, uint16_t port);
	

private:

	/**
	 * Reference of the map defined in the monitor class 
	 */
	map<uint32_t, struct OsdStat>& _osdStatMap;
};

#endif
