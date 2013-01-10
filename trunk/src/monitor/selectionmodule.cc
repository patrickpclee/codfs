#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <iterator>
#include "../config/config.hh"
#include "../common/debug.hh"
#include "selectionmodule.hh"

using namespace std;

extern ConfigLayer* configLayer;
extern mutex osdLBMapMutex;


#ifdef RR_DISTRIBUTE
mutex RRprimaryMutex;
uint32_t RRprimaryCount;
mutex RRsecondaryMutex;
uint32_t RRsecondaryCount;

#endif

SelectionModule::SelectionModule(map<uint32_t, struct OsdStat>& mapRef,
		map<uint32_t, struct OsdLBStat>& lbRef):
	_osdStatMap(mapRef), _osdLBMap(lbRef) { 
#ifdef RR_DISTRIBUTE
		RRprimaryCount = 0;
		RRsecondaryCount = 0;
#endif
	}

////////////////////////////////////////////
// 			OLD VERSION 				  //
////////////////////////////////////////////
vector<uint32_t> SelectionModule::ChoosePrimaryOld(uint32_t numOfObjs){
	//Just random choose primary
	vector<uint32_t> primaryList;
	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE) {
				// choose this as a primary
				allOnlineList.push_back(entry.first);
			}
		}
	}
	uint32_t n = allOnlineList.size();
	set<uint32_t> selected;
	while (numOfObjs) {
#ifdef RR_DISTRIBUTE
		RRprimaryMutex.lock();
		int idx = RRprimaryCount;
		RRprimaryCount = (RRprimaryCount + 1) % n;
		RRprimaryMutex.unlock();
#else
		int idx = rand() % n;
#endif
		if (selected.size() == n || 
				selected.find(allOnlineList[idx]) == selected.end()) {
			primaryList.push_back(allOnlineList[idx]);
			selected.insert(allOnlineList[idx]);
			numOfObjs --;
		}
	}
	return primaryList;
}

vector<struct BlockLocation> SelectionModule::ChooseSecondary(uint32_t numOfSegs, 
		uint32_t primaryId) {

	vector<struct BlockLocation> secondaryList;

	// Push the primary osd as the first secondary
	struct BlockLocation primary;
	primary.osdId = primaryId;
	primary.blockId = 0;
	secondaryList.push_back(primary);

	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE) {
				// choose this as a primary
				allOnlineList.push_back(entry.first);
			}
		}
	}
	set<uint32_t> selected;
	selected.insert(primaryId);
	uint32_t n = allOnlineList.size();
	numOfSegs--;
	while (numOfSegs) {
#ifdef RR_DISTRIBUTE
		RRsecondaryMutex.lock();
		int idx = RRsecondaryCount;
		RRsecondaryCount = (RRsecondaryCount + 1) % n;
		RRsecondaryMutex.unlock();
#else
		int idx = rand() % n;
#endif
		if (selected.size() == n || 
				selected.find(allOnlineList[idx]) == selected.end()) {
			struct BlockLocation tmp;
			tmp.osdId = allOnlineList[idx];
			tmp.blockId = 0;
			secondaryList.push_back(tmp);
			selected.insert(allOnlineList[idx]);
			numOfSegs--;
		}
	}
	return secondaryList;
}

/////////////////////////////////////////////////
//       NEW IMPLEMENTATION OF GREEDY ALG      //
/////////////////////////////////////////////////

vector<uint32_t> SelectionModule::ChoosePrimary(uint32_t numOfSegs) {

	// Get all online osd list
	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE) {
				allOnlineList.push_back(entry.first);
			}
		}
	}
	vector<uint32_t> primaryList;
	{
		lock_guard<mutex> lk(osdLBMapMutex);
		for (uint32_t i = 0; i < allOnlineList.size(); i++) 
			for (uint32_t j = i+1; j < allOnlineList.size(); j++)
			{
				if (_osdLBMap[allOnlineList[i]].primaryCount > 
					_osdLBMap[allOnlineList[j]].primaryCount) {
					swap (allOnlineList[i], allOnlineList[j]);
				}
			}
		for (uint32_t i = 0; i < numOfSegs; i++) {
			uint32_t id = allOnlineList[i%(allOnlineList.size())];
			primaryList.push_back(id);
			_osdLBMap[id].primaryCount++;
		}
	}
	return primaryList;
}

vector<struct BlockLocation> SelectionModule::ChooseSecondary(uint32_t 
		numOfBlks, uint32_t	primary, uint64_t blkSize) {

	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE && entry.first != primary) {
				allOnlineList.push_back(entry.first);
			}
		}
	}

	vector<struct BlockLocation> secondaryList;

	// Push the primary osd as the first secondary
	struct BlockLocation tmp;
	tmp.osdId = primary;
	tmp.blockId = 0;
	secondaryList.push_back(tmp);
	numOfBlks--;

	{
		lock_guard<mutex> lk(osdLBMapMutex);
		for (uint32_t i = 0; i < allOnlineList.size(); i++) 
			for (uint32_t j = i+1; j < allOnlineList.size(); j++)
			{
				if (_osdLBMap[allOnlineList[i]].diskCount > 
					_osdLBMap[allOnlineList[j]].diskCount) {
					swap (allOnlineList[i], allOnlineList[j]);
				}
			}
		allOnlineList.push_back(primary);
		for (uint32_t i = 0; i < numOfBlks; i++) {
			uint32_t id = allOnlineList[i%(allOnlineList.size())];
			tmp.osdId = id;
			tmp.blockId = 0;
			secondaryList.push_back(tmp);
			_osdLBMap[id].diskCount += blkSize;
		}
	}

	return secondaryList;
}

void SelectionModule::addNewOsdToLBMap(uint32_t osdId) {
	lock_guard<mutex> lk(osdLBMapMutex);
	if (_osdLBMap.find(osdId) == _osdLBMap.end()) {
		_osdLBMap[osdId] = OsdLBStat(0, 0);
	} else {
		debug ("Error: Osd %" PRIu32 " already exists in map", osdId);
	}
	//TODO
	/*
	 * Request MDS to obtain current primary number and disk utility
	 * Currently, each start up is defined as a new machine.
	 */
}
