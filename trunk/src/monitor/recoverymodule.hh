#ifndef __RECOVERYMODULE_HH__
#define __RECOVERYMODULE_HH__

#include <stdint.h>
#include "../common/osdstat.hh"
#include "../common/debug.hh"
#include "monitor_communicator.hh"
#include <map>

using namespace std;

class RecoveryModule {
public:
	RecoveryModule(map<uint32_t, struct OsdStat>& mapRef, MonitorCommunicator*
		communicator);

	void failureDetection(uint32_t deadPeriod, uint32_t sleelPeriod);

	void executeRecovery(vector<uint32_t>& deadOsdList);

	void replaceFailedOsd(struct ObjectLocation& ol, struct ObjectRepairInfo& ret);

private:
	map<uint32_t, struct OsdStat>& _osdStatMap;
	MonitorCommunicator* _communicator;
};
#endif
