#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "../config/config.hh"
#include "selectionmodule.hh"

extern ConfigLayer* configLayer;

SelectionModule::SelectionModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { 
	_numberOfOsd = configLayer->getConfigInt("Components>OSD>count");	
}

vector<uint32_t> SelectionModule::ChoosePrimary(uint32_t numOfObjs){
	//HARDCODE NOW, SHOULD BE ALGORITHMS-INVOLVED..
	vector<uint32_t> primaryList;
	for(uint32_t i = 0; i < numOfObjs; ++i)
		primaryList.push_back(52000 + (rand() % _numberOfOsd));
	return primaryList;
}

vector<struct SegmentLocation> SelectionModule::ChooseSecondary(uint32_t numOfSegs){
	//HARDCODE NOW, SHOULD BE ALGORITHMS-INVOLVED..
	vector<struct SegmentLocation> secondaryList;
	for (uint32_t i = 0; i < numOfSegs; i++) {
		struct SegmentLocation segmentLocation;

		segmentLocation.osdId = rand() % _numberOfOsd + 52000;
		cout << "OSD ID = " << segmentLocation.osdId << endl;

		segmentLocation.segmentId = 0;
		secondaryList.push_back(segmentLocation);
	}
	return secondaryList;
}
