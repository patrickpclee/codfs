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
