#ifndef __RECOVERYMODULE_HH__
#define __RECOVERYMODULE_HH__

#include <stdint.h>
#include "../common/osdstat.hh"
#include <map>

using namespace std;

class RecoveryModule {
public:
	RecoveryModule(map<uint32_t, struct OsdStat>& mapRef);

	void failureDetection(uint32_t deadPeriod, uint32_t sleelPeriod);

	void reportOsdFailure (uint32_t osdId, uint32_t health);

	uint32_t* getObjectIdList(uint32_t osdId);

	uint32_t* doRecovery (struct ObjectOwnership* objectOwnershipList);

private:
	map<uint32_t, struct OsdStat>& _osdStatMap;
};
#endif
