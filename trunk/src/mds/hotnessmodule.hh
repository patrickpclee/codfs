#ifndef __HOTNESS_MODULE_HH__
#define __HOTNESS_MODULE_HH__

#include <stdint.h>
#include <map>
#include <vector>
#include <algorithm>
#include "../common/hotness.hh"
using namespace std;

class HotnessModule {

	public:

		/*
		 * @brief Constructor
		 */
		HotnessModule();


		/* 
		 * @brief Interface for other to call, update the hotness for a segment
		 *
		 * @param segmentId Update target
		 * @param alg Algorithm for hotness update
		 * @param idx index of segmentId, 0 for the original one, 1 is the next
		 * segment of the segment being read, 2 is the next of next...
		 *
		 * @return a struct cantains the number of new cache still required and
		 * the list of already cached osd
		 */
		struct HotnessRequest updateSegmentHotness(uint64_t segmentId, 
				enum HotnessAlgorithm alg, uint32_t idx);

		/*
 		 * @brief When a osd build a cahce for a segment, call this to update
		 * the cache list of that segment in hotness module
		 * 
		 * @param segmentId ID of target segment
		 * @param cachedOsd ID of target Osd that newly cache this segment
		 */
		void updateSegmentCache(uint64_t segmentId, uint32_t cachedOsd);

		/*
 		 * @brief When a osd build a cahce for a segment, call this to update
		 * the cache list of that segment in hotness module
		 * 
		 * @param segmentId ID of target segment
		 * @param cachedOsdList List of target OSDs that newly cache this segment
		 */
		void updateSegmentCache(uint64_t segmentId, vector<uint32_t> cachedOsdList);

		/*
 		 * @brief Used when periodical synchronize of cachelist between OSD and
		 * MDS, delete the cache list in the hotness module
		 * 
		 * @param osdId ID of OSD sent this sync message
		 * @param segmentIdList List of segmentIds that this Osd has delete 
		 * from its cache
		 */
		void deleteSegmentCache(uint32_t osdId, vector<uint64_t> segmentIdList);
		void deleteSegmentCache(uint64_t segmentId);

		/*
 		 * @brief Get function for cache entry of a particular segmentId
		 *
		 * @param segmentId Target segmentId
		 *
		 * @return return a vector of cached OSD list
		 */
		vector<uint32_t>  getSegmentCacheEntry(uint64_t segmentId);

	private:

		/*
 		 * @brief Used when hotness is updated, check whether there is enough
		 * cache, if not, produce a request for MDS to create more cache
		 * 
		 * @param segmentId Target segment ID
		 * @param type Hotness type of that segment
		 *
		 * @return A struct of HotnessRequest that contains the number of new
		 * OSD needed for cache and the already cached OSD list
		 */
		struct HotnessRequest checkHotnessCache(uint64_t segmentId, enum HotnessType
				type);


		/*
 		 * @brief Get function for hotness entry of a particular segmentId
		 *
		 * @param segmentId Target segmentId
		 * 
		 * @return return a Hotness structure
		 */
		struct Hotness getSegmentHotnessEntry(uint64_t segmentId);

		/*
 		 * @brief Set function for hotness entry of a particular segmentId
		 *
		 * @param segmentId Target segmentId
		 * @param newHotness New struct Hotness values
		 */
		void setSegmentHotnessEntry(uint64_t segmentId, struct Hotness newHotness);

		/*
 		 * @brief Default algorithm for hotness value update
		 *
		 * @param oldHotness Original hotness struct
		 * @param newHotness Reference for new hotness struct
		 */
		void defaultHotnessUpdate(const struct Hotness& oldHotness, 
				struct Hotness& newHotness);

		/*
 		 * @brief Top k algorithm for hotness value update
		 *
		 * @param oldHotness Original hotness struct
		 * @param newHotness Reference for new hotness struct
		 * @param topK Select top K access segment for update type VERY HOT
		 * @param topK2 Select top K2 access segment for update type HOT
		 */
		void topHotnessUpdate(const struct Hotness& oldHotness, 
				struct Hotness& newHotness, uint32_t topK, uint32_t topK2);



		/*
 		 * @brief Map structure for storing the hotness
		 */
		map<uint64_t, struct Hotness> _hotnessMap;

		/*
 		 * @brief Map structure for storing the cache list
		 */
		map<uint64_t, vector<uint32_t> > _cacheMap;
};

#endif
