#include <ctime>
#include <stdio.h>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <set>
#include <thread>
#include <algorithm>
#include "recoverymodule.hh"
#include "../protocol/status/recoverytriggerrequest.hh"
#include "../protocol/status/repairsegmentinfomsg.hh"
using namespace std;

RecoveryModule::RecoveryModule(map<uint32_t, struct OsdStat>& mapRef,
		MonitorCommunicator* communicator):
	_osdStatMap(mapRef), _communicator(communicator) { 

	}

void startRecoveryProcedure(RecoveryModule* rm, vector<uint32_t> deadOsdList,
		bool dstSpecified, vector<uint32_t> dstSpec) {
	string osdListString;
	for (auto osdId : deadOsdList) {
		osdListString += to_string(osdId) + " ";
	}
	cout << "Start to recover failure OSD " << osdListString << endl;
	rm->executeRecovery(deadOsdList, dstSpecified, dstSpec);
}


// Just sequentially choose one.
// Can use different startegy later.
void RecoveryModule::replaceFailedOsd(struct SegmentLocation& ol,
		struct SegmentRepairInfo& ret, map<uint32_t, uint32_t>& mapped) {

	lock_guard<mutex> lk(osdStatMapMutex);
	ret.segmentId = ol.segmentId;

	vector<uint32_t>& ref = ol.osdList;
	for (int pos = 0; pos < (int)ref.size(); ++pos) {
		if (_osdStatMap[ref[pos]].osdHealth != ONLINE) {
			uint32_t deadOsd = ref[pos];
			// mapped this dead osd with new replacement
			ref[pos] = mapped[deadOsd];
			ret.repPos.push_back(pos);
			ret.repOsd.push_back(mapped[deadOsd]);
		}
	}

}
void RecoveryModule::replaceFailedOsd(struct SegmentLocation& ol, struct SegmentRepairInfo& ret) {
	// if not specify the dst, just sequential assign
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

		ret.segmentId = ol.segmentId;

		vector<uint32_t>& ref = ol.osdList;
		for (int pos = 0; pos < (int)ref.size(); ++pos) {
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

void RecoveryModule::executeRecovery(vector<uint32_t>& deadOsdList, bool
		dstSpecified, vector<uint32_t> dstSpec) {
	// this map used if dstSpecified
	map<uint32_t, uint32_t> mapped;
	if (dstSpecified) {
		// one to one map from dead to dst
		sort(deadOsdList.begin(), deadOsdList.end());
		sort(dstSpec.begin(), dstSpec.end());
		for (int i = 0; i < (int)deadOsdList.size(); i++) {
			mapped[deadOsdList[i]] = dstSpec[i%dstSpec.size()];
		}
	}

	debug_yellow("%s\n", "Start Recovery Procedure");

	// Request Recovery to Mds
	RecoveryTriggerRequestMsg* rtrm = new
		RecoveryTriggerRequestMsg(_communicator, _communicator->getMdsSockfd(),
				deadOsdList, dstSpecified, dstSpec); // add two fields dstSpec
	rtrm->prepareProtocolMsg();
	_communicator->addMessage(rtrm, true);

	MessageStatus status = rtrm->waitForStatusChange();
	if (status == READY) {
		vector<struct SegmentLocation> ols = rtrm->getSegmentLocations();
		for (struct SegmentLocation ol: ols) {
			// Print debug message
			debug_cyan("Segment Location id = %" PRIu64 " primary = %" PRIu32 "\n", ol.segmentId, 
					ol.primaryId);
			
			struct SegmentRepairInfo ori;
			if (dstSpecified) {
				replaceFailedOsd (ol, ori, mapped);	
			} else {
				replaceFailedOsd (ol, ori);	
			}
			ori.out();

			RepairSegmentInfoMsg* roim = new RepairSegmentInfoMsg(_communicator,
					_communicator->getSockfdFromId(ol.primaryId), ori.segmentId,
					ori.repPos, ori.repOsd);
			debug ("sockfd for repair = %" PRIu32 "\n", _communicator->getSockfdFromId(ol.primaryId));
			roim->prepareProtocolMsg();
			debug ("%s\n", "add repair segment info msg");
			_communicator->addMessage(roim);
			debug ("%s\n", "added to queue repair segment info msg");
		}

	} else {
		debug("%s\n", "Faided Recovery");
	}

}

void RecoveryModule::failureDetection(uint32_t deadPeriod, uint32_t sleepPeriod) {
	while (1) {
		{
			lock_guard<mutex> lk(triggerRecoveryMutex);
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
				thread recoveryProcedure(startRecoveryProcedure, this, deadOsdList, false, vector<uint32_t>());
				recoveryProcedure.detach();
			}
		}
		sleep(sleepPeriod);
	}
}

void RecoveryModule::userTriggerDetection(bool dstSpecified) {
	lock_guard<mutex> lk(triggerRecoveryMutex);
	vector<uint32_t> deadOsdList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			//entry.second.out();
			if ((entry.second.osdHealth != RECOVERING &&
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
		vector<uint32_t> dstSpec;
		if (dstSpecified) {
			FILE* fp = fopen(RECOVERY_DST, "r");
			int osdid; 
			while (fscanf(fp, "%d", &osdid) != EOF) {
				dstSpec.push_back(osdid);
			}
		}
		thread recoveryProcedure(startRecoveryProcedure, this, deadOsdList,
				dstSpecified, dstSpec);
		recoveryProcedure.detach();
	}
}

