#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>
#include <vector>
#include <map>
#include "../common/osdstat.hh"
#include "../osd/segmentlocation.hh"
#include "../protocol/status/osdstatupdaterequestmsg.hh"

using namespace std;

class SelectionModule {
public:

	SelectionModule(map<uint32_t, struct OsdStat>& mapRef);
	uint32_t findNextOsd(); // osd to store the next object
	uint32_t* findSecondaryOsdList(uint32_t splitFactor);
	uint32_t* findPrimaryOsdList(uint32_t numOfOsd);
	uint32_t findRecoveryOsd(struct OjbectOwnership objectOwnership);
	vector<uint32_t> ChoosePrimary(uint32_t numOfObjs);
	vector<struct SegmentLocation> ChooseSecondary(uint32_t numOfSegs);

private:
	map<uint32_t, struct OsdStat>& _osdStatMap;
};

#endif
