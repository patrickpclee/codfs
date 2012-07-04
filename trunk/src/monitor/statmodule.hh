#ifndef __STATMODULE_HH__
#define __STATMODULE_HH__

#include <stdint.h>

class StatModule {

public:
	void pollAndUpdateStat ();
//	void insertStat (struct OsdStat newStat);
	void removeStatById (uint32_t osdId);
	void setHealthById (uint32_t osdId, uint32_t health);
	void setCapacityById (uint32_t osdId, uint32_t capacity);
	void setLoadingById (uint32_t osdId, uint32_t loading);
private:

};

#endif
