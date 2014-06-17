#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>
#include <vector>
#include <map>
#include "../common/osdstat.hh"
#include "../common/blocklocation.hh"
#include "../protocol/status/osdstatupdaterequestmsg.hh"

using namespace std;

class SelectionModule {

	public:

		/**
		 * Constructor
		 * @param mapRef Reference of the osd status map in monitor class
		 */
		SelectionModule(map<uint32_t, struct OsdStat>& mapRef, map<uint32_t, struct
				OsdLBStat>& lbRef);

		/**
		 * Choose primary osds from the osd status map
		 * @param numOfSegs Number of OSDs going to be selected
		 * @return a list of selected osd IDs  
		 */
		vector<uint32_t> choosePrimary(uint32_t numOfSegs);


		/**
		 * Choose secondary osds from the osd status map to store coded blocks
		 * @param numOfBlks Number of OSDs going to be selected
		 * @param primary Primary OSD id for this segment
		 * @param blkSize Each blk size of the encoded segment
		 * @return a list of selected osd IDs  
		 */
		vector<struct BlockLocation> chooseSecondary(uint32_t numOfBlks, uint32_t
				primary, uint64_t blkSize);

		/**
		 * Add a newly startup osd to the load balancing map
		 * @param osdId Newly startup Osd Id
		 */
		void addNewOsdToLBMap(uint32_t osdId);

	private:

		/**
		 * References of the maps defined in the monitor class 
		 */
		map<uint32_t, struct OsdStat>& _osdStatMap;
		map<uint32_t, struct OsdLBStat>& _osdLBMap;

};

#endif
