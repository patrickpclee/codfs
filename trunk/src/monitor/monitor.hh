#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "../cache/cache.hh"
#include "../common/osdstat.hh"
#include "monitor_communicator.hh"
#include "selectionmodule.hh"
#include "recoverymodule.hh"
#include "statmodule.hh"

class Monitor {

public:
	// handle requests from MDS / OSD
	uint32_t* primaryOsdListHandler(uint32_t objectCount);
	uint32_t* secondaryOsdListHandler();
	void osdFailureHandler(uint32_t osdId);

	// triggered by timeout
	void osdRecoveryRequest(uint32_t osdId);
	void recoveryResultHandler(uint32_t osdId, bool success);

	MonitorCommunicator* getCommunicator();

private:
//	MonitorInfo _info;
	Cache _cache;
	MonitorCommunicator* _monitorCommunicator;

	SelectionModule* _selectionModule;
	RecoveryModule* _recoveryModule;
	StatModule* _statModule;

//	struct OsdStat [] osdStat;
	vector<struct OsdStat> _osdStats;
};

#endif
