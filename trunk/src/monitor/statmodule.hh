#ifndef __STATMODULE_HH__
#define __STATMODULE_HH__

#include <stdint.h>
#include "../common/osdstat.hh"
#include "../communicator/communicator.hh"
#include <map>

using namespace std;

class StatModule {

public:

	StatModule(map<uint32_t, struct OsdStat>& mapRef);

	void updateOsdStatMap (Communicator* communicator);

	/*  remove an osd status entry by its osdId 
	 */
	void removeStatById (uint32_t osdId);

	/*  set an osd status entry, if not in the map, create it
	 *  else update the result
	 */
	void setStatById (uint32_t osdId, uint32_t capacity, uint32_t loading,
		 uint32_t health);
	

private:
	map<uint32_t, struct OsdStat>& _osdStatMap;
};

#endif
