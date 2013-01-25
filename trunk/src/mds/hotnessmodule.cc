#include <mutex>
#include <thread>
#include "../common/debug.hh"
#include "hotnessmodule.hh"

const double HOUR12 = 60 * 60 * 12.0;
const double NEWUPDATE = 10.0;
const double NEWUPDATE15 = 10 * 1.5;
const double NEWUPDATE25 = 10 * 2.5;
const uint32_t REPLICA[3] = {0, 1, 3};

mutex hotnessMapMutex;
mutex cacheMapMutex;
// Function Implementation go here

/*
 * @brief Default constructor
 */
HotnessModule::HotnessModule() {
	_hotnessMap.clear();
	_cacheMap.clear();
}

/*
 * @brief return the number of cache still required and the used cache osd list
 */
struct HotnessRequest HotnessModule::updateSegmentHotness(uint64_t segmentId,
		HotnessAlgorithm algo, uint32_t idx) {
	struct Hotness oldHotness = getSegmentHotnessEntry(segmentId);
	struct Hotness newHotness;
	switch (algo) {
		case DEFAULT_HOTNESS_ALG:
			defaultHotnessUpdate(oldHotness, newHotness);
			break;
		default:
			break;
	}
	setSegmentHotnessEntry(segmentId, newHotness);
	{
		lock_guard<mutex> lk(hotnessMapMutex);
		debug_yellow("Hotness of SegmentId %" PRIu64 " Value = %lf and Type = %" PRIu32 "\n", segmentId, _hotnessMap[segmentId].hotness, _hotnessMap[segmentId].type);
	}
	return checkHotnessCache(segmentId, newHotness.type);
}

/*
 * @brief update the cache list for a particular segment with single osd
 */
void HotnessModule::updateSegmentCache(uint64_t segmentId, uint32_t cachedOsd) {
	lock_guard<mutex> lk(cacheMapMutex);
	map <uint64_t, vector<uint32_t> >::iterator it = _cacheMap.find(segmentId);
	if (it == _cacheMap.end()) {
		_cacheMap[segmentId] = vector<uint32_t>(1, cachedOsd);
	} else {
		vector<uint32_t>& listRef = it->second;
		listRef.push_back(cachedOsd);
		sort (listRef.begin(), listRef.end());
		vector<uint32_t>::iterator it = unique(listRef.begin(), listRef.end());
		listRef.resize (it - listRef.begin());
	}
	return;
}

/*
 * @brief update the cache list for a particular segment with list of osd
 */
void HotnessModule::updateSegmentCache(uint64_t segmentId, vector<uint32_t> cachedOsdList) {
	lock_guard<mutex> lk(cacheMapMutex);
	map <uint64_t, vector<uint32_t> >::iterator it = _cacheMap.find(segmentId);
	if (it == _cacheMap.end()) {
		_cacheMap[segmentId] = vector<uint32_t>(cachedOsdList);
	} else {
		vector<uint32_t>& listRef = it->second;
		listRef.insert(listRef.end(), cachedOsdList.begin(), cachedOsdList.end());
		sort (listRef.begin(), listRef.end());
		vector<uint32_t>::iterator it = unique(listRef.begin(), listRef.end());
		listRef.resize (it - listRef.begin());
	}
	return;
}


/*
 * @brief delete the osdId from the cache list for a set of segmentIds
 */
void HotnessModule::deleteSegmentCache(uint32_t osdId, vector<uint64_t> segmentIdList) {
	lock_guard<mutex> lk(cacheMapMutex);
	for (uint64_t segmentId: segmentIdList) {
		map <uint64_t, vector<uint32_t> >::iterator it = _cacheMap.find(segmentId);
		if (it == _cacheMap.end()) {
			// nothing to delete
			return;
		} else {
			vector<uint32_t>& listRef = it->second;
			vector<uint32_t>::iterator eraseIt = find(listRef.begin(), listRef.end(), osdId);
			if (eraseIt != listRef.end()) {
				listRef.erase(eraseIt);
			}
		}
	}
	return;
}

void HotnessModule::deleteSegmentCache(uint64_t segmentId) {
	lock_guard<mutex> lk(cacheMapMutex);
	_cacheMap.erase(segmentId);
}

/*
 * @brief newHotness = F(currentTime - lastUpdateTime) * oldHotness + constant
 * where F is a quadratic function f(x) = 1-x^2, for x in (0, 1)
 * timestamp difference is normalized by 12 hours 
 * HOTTEST:  hotness > NEWUPDATE * 2.5
 * HOT: hotness > NEWUPDATE * 1.5
 */
void HotnessModule::defaultHotnessUpdate(const struct Hotness& oldHotness, 
		struct Hotness& newHotness) {

	uint32_t currentTS = time(NULL);
	double x = (currentTS - oldHotness.timestamp) / HOUR12;
	// update timestamp
	newHotness.timestamp = currentTS;

	// update hotness value
	if (x > 1) {
		newHotness.hotness = NEWUPDATE;
	} else {
		newHotness.hotness =  (1 - x*x) * oldHotness.hotness + NEWUPDATE;
	}

	// update hotness type
	if (newHotness.hotness > NEWUPDATE25) {
		newHotness.type = HOTTEST;
	} else 
		if (newHotness.hotness > NEWUPDATE15) {
			newHotness.type = HOT;
		} else {
			newHotness.type = COLD;
		}

}

struct Hotness HotnessModule::getSegmentHotnessEntry(uint64_t segmentId) {
	lock_guard<mutex> lk(hotnessMapMutex);
	map <uint64_t, struct Hotness>::iterator it = _hotnessMap.find(segmentId);
	if (it == _hotnessMap.end()) {
		_hotnessMap[segmentId] = Hotness();
		return Hotness();
	} else {
		return it->second;
	}
	return Hotness();
}

void HotnessModule::setSegmentHotnessEntry(uint64_t segmentId, struct Hotness
		newHotness) {
	lock_guard<mutex> lk(hotnessMapMutex);
	_hotnessMap[segmentId] = newHotness;
}

vector<uint32_t> HotnessModule::getSegmentCacheEntry(uint64_t segmentId) {
	lock_guard<mutex> lk(cacheMapMutex);
	map <uint64_t, vector<uint32_t> >::iterator it = _cacheMap.find(segmentId);
	if (it == _cacheMap.end()) {
		_cacheMap[segmentId] = vector<uint32_t>();
		return vector<uint32_t>();
	} else {
		return it->second;
	}
	return vector<uint32_t>();	
}

struct HotnessRequest HotnessModule::checkHotnessCache(uint64_t segmentId, 
		enum HotnessType type) {

	struct HotnessRequest req;
	req.cachedOsdList = getSegmentCacheEntry(segmentId);
	if (REPLICA[type] <= req.cachedOsdList.size()) {
		req.numOfNewCache = 0;
	} else {
		req.numOfNewCache = REPLICA[type] - req.cachedOsdList.size();
	}
	return req;
}
