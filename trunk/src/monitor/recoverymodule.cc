#include <ctime>
#include <stdio.h>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <set>
#include <thread>
#include "recoverymodule.hh"
#include "../protocol/status/recoverytriggerrequest.hh"
#include "../protocol/status/repairobjectinfomsg.hh"
using namespace std;

RecoveryModule::RecoveryModule(map<uint32_t, struct OsdStat>& mapRef,
		MonitorCommunicator* communicator):
	_osdStatMap(mapRef), _communicator(communicator) { }

	void startRecoveryProcedure(RecoveryModule* rm, vector<uint32_t> deadOsdList) {
		rm->executeRecovery(deadOsdList);
	}


// Just sequentially choose one.
// Can use different startegy later.
void RecoveryModule::replaceFailedOsd(struct ObjectLocation& ol, struct ObjectRepairInfo& ret) {
	set<uint32_t> used;
	for (uint32_t osdid : ol.osdList) {
		if (_osdStatMap[osdid].osdHealth == ONLINE) {
			used.insert(osdid);
		}
	}

	set<uint32_t> avail;
	set<uint32_t> all;

	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for (auto& entry : _osdStatMap) {
			if (entry.second.osdHealth == ONLINE) {
				all.insert(entry.first);
				if (!used.count(entry.first))
					avail.insert(entry.first);
			}
		}

		set<uint32_t>::iterator it = avail.begin();

		ret.objectId = ol.objectId;

		vector<uint32_t>& ref = ol.osdList;
		for (int pos = 0; pos < ref.size(); ++pos) {
			if (_osdStatMap[ref[pos]].osdHealth != ONLINE) {
				if (it == avail.end() || it == all.end()) it = all.begin();
				if (it != avail.end() && it != all.end()) {
					debug_cyan("Faild osd = %" PRIu32 "\n", ref[pos]);
					ref[pos] = *it;
					ret.repPos.push_back(pos);
					ret.repOsd.push_back(*it);
					debug_cyan("Replaced with osd = %" PRIu32 "\n", ref[pos]);
					it++;
				} else {
					debug_cyan("%s\n", "[ERROR]: Failed to replace osd");
				}
			}
		}
	}
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
			debug_cyan("Object Location id = %" PRIu64 " primary = %" PRIu32 "\n", ol.objectId, 
					ol.primaryId);
			/*
			   printf ("--------Original OSD----------------\n");
			   for (uint32_t osdid : ol.osdList) printf("%d ", osdid);
			   printf("\n--------Reverse OSD-------------\n");

			   for (uint32_t osdid : ol.osdList) printf("%d ", osdid);
			   printf("\n------------------------------------\n");
			 */
			struct ObjectRepairInfo ori;
			replaceFailedOsd (ol, ori);	
			ori.out();

			RepairObjectInfoMsg* roim = new RepairObjectInfoMsg(_communicator,
					_communicator->getSockfdFromId(ol.primaryId), ori.objectId,
					ori.repPos, ori.repOsd);
			roim->prepareProtocolMsg();
			_communicator->addMessage(roim);
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
				//entry.second.out();
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

