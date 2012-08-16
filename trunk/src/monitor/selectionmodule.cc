#include <iostream>
#include <cstdio>
#include "selectionmodule.hh"


SelectionModule::SelectionModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { }

vector<uint32_t> SelectionModule::ChoosePrimary(uint32_t numOfObjs){
	//HARDCODE NOW, SHOULD BE ALGORITHMS-INVOLVED..
	vector<uint32_t> primaryList;
	for(uint32_t i = 0; i < numOfObjs; ++i)
		primaryList.push_back(52000 + (i % 2));
	return primaryList;
}

vector<struct SegmentLocation> SelectionModule::ChooseSecondary(uint32_t numOfSegs){
	//HARDCODE NOW, SHOULD BE ALGORITHMS-INVOLVED..
	vector<struct SegmentLocation> secondaryList;
	for (uint32_t i = 0; i < numOfSegs; i++) {
		struct SegmentLocation segmentLocation;

		segmentLocation.osdId = rand() % 2 + 52000;
		cout << "OSD ID = " << segmentLocation.osdId << endl;

		/*
		 if (_componentId == 52000)
		 segmentLocation.osdId = 52001;
		 else
		 segmentLocation.osdId = 52000;
		 */

		segmentLocation.segmentId = 0;
		secondaryList.push_back(segmentLocation);
	}
	return secondaryList;
}
