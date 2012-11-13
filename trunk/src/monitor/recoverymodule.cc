#include <ctime>
#include <stdio.h>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <thread>
#include "recoverymodule.hh"
#include "../protocol/status/recoverytriggerrequest.hh"
using namespace std;

RecoveryModule::RecoveryModule(map<uint32_t, struct OsdStat>& mapRef,
		MonitorCommunicator* communicator):
	_osdStatMap(mapRef), _communicator(communicator) { }

	void startRecoveryProcedure(RecoveryModule* rm, vector<uint32_t> deadOsdList) {
		rm->executeRecovery(deadOsdList);
	}

void RecoveryModule::executeRecovery(vector<uint32_t>& deadOsdList) {
	debug_yellow("%s\n", "Start Recovery Procedure");

	// Request Recovery to Mds
	RecoveryTriggerRequestMsg* rtrm = new
		RecoveryTriggerRequestMsg(_communicator, _communicator->getMdsSockfd(),
				deadOsdList);
	rtrm->prepareProtocolMsg();
	_communicator->addMessage(rtrm, true);

	MessageStatus status = rtrm->waitForStatusChange();
	if (status == READY) {
		vector<struct ObjectLocation> ols = rtrm->getObjectLocations();
		for (struct ObjectLocation ol: ols) {
			// Print debug message
			debug_cyan("Object Location id = %" PRIu64 " primary = %" PRIu32 " osdlist = [", ol.objectId, 
				ol.primaryId);

			


		}

	} else {
		debug("%s\n", "Faided Recovery");
	}

}

void RecoveryModule::failureDetection(uint32_t deadPeriod, uint32_t sleepPeriod) {
	while (1) {
		uint32_t currentTS = time(NULL);
		vector<uint32_t> deadOsdList;
		{
			lock_guard<mutex> lk(osdStatMapMutex);
			for(auto& entry: _osdStatMap) {
				entry.second.out();
				if ((currentTS - entry.second.timestamp) > deadPeriod &&
						(entry.second.osdHealth != RECOVERING &&
						 entry.second.osdHealth != ONLINE)) {
					// Trigger recovery 
					debug_yellow("Detect failure OSD = %" PRIu32 "\n", entry.first);
					deadOsdList.push_back (entry.first);
					entry.second.osdHealth = RECOVERING;
				}
			}
		}
		// If has dead Osd, start recovery
		if (deadOsdList.size() > 0) {
			thread recoveryProcedure(startRecoveryProcedure, this, deadOsdList);
			recoveryProcedure.detach();
		}
		sleep(sleepPeriod);
	}
}

