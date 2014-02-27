#ifndef __RECOVERYMODULE_HH__
#define __RECOVERYMODULE_HH__

#include <stdint.h>
#include "../common/osdstat.hh"
#include "../common/debug.hh"
#include "monitor_communicator.hh"
#include <map>
#include <mutex>

using namespace std;

class RecoveryModule {
	public:
		RecoveryModule(map<uint32_t, struct OsdStat>& mapRef, MonitorCommunicator*
				communicator);

		void failureDetection(uint32_t deadPeriod, uint32_t sleelPeriod);

		void userTriggerDetection(bool dstSpecified = false);

		void executeRecovery(vector<uint32_t>& deadOsdList, bool dstSpecified,
				vector<uint32_t> dstSpec);

		void replaceFailedOsd(struct SegmentLocation& ol, struct SegmentRepairInfo& ret,
				map<uint32_t, uint32_t>& mapped);

		void replaceFailedOsd(struct SegmentLocation& ol, struct SegmentRepairInfo& ret);

	private:
		map<uint32_t, struct OsdStat>& _osdStatMap;
		MonitorCommunicator* _communicator;
		mutex triggerRecoveryMutex;
};
#endif
