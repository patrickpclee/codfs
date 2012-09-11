#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <set>
#include "../config/config.hh"
#include "../common/debug.hh"
#include "selectionmodule.hh"

using namespace std;

extern ConfigLayer* configLayer;

SelectionModule::SelectionModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { 
}

vector<uint32_t> SelectionModule::ChoosePrimary(uint32_t numOfObjs){
	//Just random choose primary
	vector<uint32_t> primaryList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		while (numOfObjs>0) {
			for(auto& entry: _osdStatMap) {
				if (entry.second.osdHealth == ONLINE && (rand()&1)) {
					// choose this as a primary
					primaryList.push_back(entry.first);
					--numOfObjs;
					if (!numOfObjs) break;
				}
			}
		}
	}
	return primaryList;
}

vector<struct SegmentLocation> SelectionModule::ChooseSecondary(uint32_t numOfSegs){
	//Just random choose secondary
	vector<struct SegmentLocation> secondaryList;
	{
		set<uint32_t> selected;
		lock_guard<mutex> lk(osdStatMapMutex);
		while (numOfSegs>0) {
			for(auto& entry: _osdStatMap) {
				bool isAllOsdUsed = selected.size() >= _osdStatMap.size();
				if (entry.second.osdHealth == ONLINE && (isAllOsdUsed ||
							(!isAllOsdUsed && !selected.count(entry.first))) && (rand()&1)) {
					// choose this as a secondary
					struct SegmentLocation segmentLocation;
					segmentLocation.osdId = entry.first;
					segmentLocation.segmentId = 0;	// segmentId will be set by OSD
					secondaryList.push_back(segmentLocation);
					selected.insert(entry.first);
					--numOfSegs;
					if (!numOfSegs) break;
				}
			}
		}
	}
	return secondaryList;
}
