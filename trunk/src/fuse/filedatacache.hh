#ifndef __FILE_DATA_CACHE_HH__
#define __FILE_DATA_CACHE_HH__

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unordered_map>
#include <thread>

#include "../common/enums.hh"
#include "../common/segmentdata.hh"

class FileDataCache {
	public:
		FileDataCache();
		// Make use of Segment Cache at ClientStorageModule
		uint32_t readDataCache(uint64_t segmentId, uint32_t primary, void* buf, uint32_t size, uint32_t offset);

		uint32_t writeDataCache(uint64_t segmentId, uint32_t primary, const void* buf, uint32_t size, uint32_t offset);

		void closeDataCache(uint64_t segmentId);
	private:
		void writeBack(uint64_t segmentId);

		std::unordered_map<uint64_t, SegmentStatus> _segmentStatus;
		std::unordered_map<uint64_t, uint32_t> _segmentPrimary;
		std::unordered_map<uint64_t, struct SegmentData> _segmentDataCache;

		uint32_t _segmentSize;
		string _codingSetting;
		CodingScheme _codingScheme;

		std::mutex _dataCacheMutex;
		std::unordered_map<uint64_t, std::mutex*> _segmentLock;
};
#endif 
