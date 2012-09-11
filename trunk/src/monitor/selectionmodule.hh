#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>
#include <vector>
#include <map>
#include "../common/osdstat.hh"
#include "../osd/segmentlocation.hh"
#include "../protocol/status/osdstatupdaterequestmsg.hh"

using namespace std;

class SelectionModule {

public:

	/**
	 * Constructor
	 * @param mapRef Reference of the osd status map in monitor class
	 */
	SelectionModule(map<uint32_t, struct OsdStat>& mapRef);
	
	/**
	 * Choose primary osds from the osd status map
	 * @param numOfObjs Number of OSDs going to be selected
	 * @return a list of selected osd IDs  
	 */
	vector<uint32_t> ChoosePrimary(uint32_t numOfObjs);

	/**
	 * Choose secondary osds from the osd status map to store coded segments
	 * @param numOfSegs Number of OSDs going to be selected
	 * @return a list of selected osd IDs  
	 */
	vector<struct SegmentLocation> ChooseSecondary(uint32_t numOfSegs);

private:

	/**
	 * Reference of the map defined in the monitor class 
	 */
	map<uint32_t, struct OsdStat>& _osdStatMap;

};

#endif
