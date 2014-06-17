#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <iterator>
#include <algorithm>
#include "../config/config.hh"
#include "../common/debug.hh"
#include "selectionmodule.hh"

using namespace std;

extern ConfigLayer* configLayer;
extern mutex osdLBMapMutex;

const uint32_t MIN_FREE_SPACE = 100; // in MB


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


/////////////////////////////////////////////////
//       NEW IMPLEMENTATION OF GREEDY ALG      //
/////////////////////////////////////////////////

vector<uint32_t> SelectionModule::choosePrimary(uint32_t numOfSegs) {

	// Get all online osd list
	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE &&
                entry.second.osdCapacity > MIN_FREE_SPACE) {
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

vector<struct BlockLocation> SelectionModule::chooseSecondary(uint32_t 
		numOfBlks, uint32_t	primary, uint64_t blkSize) {

#ifdef RANDOM_CHOOSE_SECONDARY
    vector<uint32_t> allOnlineList;
    {
        lock_guard<mutex> lk(osdStatMapMutex);
        for(auto& entry: _osdStatMap) {
            if (entry.second.osdHealth == ONLINE && entry.first != primary
                && entry.second.osdCapacity > MIN_FREE_SPACE) {
                allOnlineList.push_back(entry.first);
            }
        }
    }

    vector<struct BlockLocation> secondaryList;
    struct BlockLocation tmp;
    tmp.osdId = primary;
    tmp.blockId = 0;
    secondaryList.push_back(tmp);
    numOfBlks--;

    random_shuffle(allOnlineList.begin(), allOnlineList.end());
    allOnlineList.push_back(primary); // ensure primary is at the end

    if (allOnlineList.size() == 0) {
        debug_error("%s", "ERROR: No harddisk is available!\n");
        exit(-1);
    }

    if (allOnlineList.size() < numOfBlks) {
        debug_error("Warning: number of available osd %zu < number of blocks %" PRIu32 "\n",
            allOnlineList.size(), numOfBlks);
    }

    int i = 0;
    while (numOfBlks > 0) {
        tmp.osdId = allOnlineList[i];
        tmp.blockId = 0;
        secondaryList.push_back(tmp);
        i++;
        numOfBlks--;

        // start repeating if no more nodes available
        if (i == (int)allOnlineList.size()) {
            i=0;
        }
    }
    return secondaryList;
#else

	vector<uint32_t> allOnlineList;
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for(auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE && entry.first != primary
                && entry.second.osdCapacity > MIN_FREE_SPACE) {
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

    if (allOnlineList.size() < numOfBlks) {
        debug_error("Warning: number of available osd %" PRIu32 " < number of blocks %" PRIu32 "\n", 
            allOnlineList.size(), numOfBlks);
    }
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
#endif
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
