#include "recoverymodule.hh"
#include <ctime>
#include <stdio.h>
#include <mutex>
#include <unistd.h>
using namespace std;

RecoveryModule::RecoveryModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { }

void RecoveryModule::failureDetection(uint32_t deadPeriod, uint32_t sleepPeriod) {
	while (1) {
		uint32_t currentTS = time(NULL);
		{
			lock_guard<mutex> lk(osdStatMapMutex);
			for(auto& entry: _osdStatMap) {
				if ((currentTS - entry.second.timestamp) > deadPeriod) {
					// Trigger recovery 
					printf("OH SHIT...\n");
				}
			}
		}
		sleep(sleepPeriod);
	}
}
