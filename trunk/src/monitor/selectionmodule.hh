#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>
#include <map>
#include "../common/osdstat.hh"

using namespace std;

class SelectionModule {
public:

	SelectionModule(map<uint32_t, struct OsdStat>& mapRef);
	uint32_t findNextOsd(); // osd to store the next object
	uint32_t* findSecondaryOsdList(uint32_t splitFactor);
	uint32_t findRecoveryOsd(struct OjbectOwnership objectOwnership);

private:
	map<uint32_t, struct OsdStat>& _osdStatMap;
};

#endif
