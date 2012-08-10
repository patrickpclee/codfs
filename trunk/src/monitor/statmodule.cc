#include "statmodule.hh"

/*  Constructor */
StatModule::StatModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { }

void StatModule::updateOsdStatMap (Communicator* communicator) {
	while (1) {
		{
			lock_guard<mutex> lk(osdStatMapMutex);
			for(const auto& entry: _osdStatMap) {
					
			}
		}
		usleep(1000000);
	}

}

void StatModule::setStatById (uint32_t osdId, uint32_t capacity, 
	uint32_t loading, uint32_t health) {

}
