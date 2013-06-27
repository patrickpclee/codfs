#ifndef __FILE_DATA_CACHE_HH__
#define __FILE_DATA_CACHE_HH__

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

#include "../common/enums.hh"
#include "../common/segmentdata.hh"

#include "../datastructure/ringbuffer.hh"

class FileDataCache {
	public:
		FileDataCache();
		// Make use of Segment Cache at ClientStorageModule
		uint32_t readDataCache(uint64_t segmentId, uint32_t primary, void* buf, uint32_t size, uint32_t offset);

		uint32_t writeDataCache(uint64_t segmentId, uint32_t primary, const void* buf, uint32_t size, uint32_t offset);

		void closeDataCache(uint64_t segmentId, bool sync = false);
		void prefetchSegment(uint64_t segmentId, uint32_t primary);
	private:
		void writeBack(uint64_t segmentId);
		void writeBackThread();
		void doWriteBack(uint64_t segmentId);

		void doPrefetch();
		void updateLru(uint64_t segmentId);

		std::unordered_map<uint64_t, SegmentStatus> _segmentStatus;
		std::unordered_map<uint64_t, uint32_t> _segmentPrimary;
		std::unordered_map<uint64_t, struct SegmentData> _segmentDataCache;

		uint32_t _segmentSize;
		string _codingSetting;
		CodingScheme _codingScheme;
		uint32_t _lruSizeLimit;

		std::mutex _dataCacheMutex;
		std::unordered_map<uint64_t, std::mutex*> _segmentLock;

		std::mutex _lruListMutex;
		std::list<uint64_t> _segmentLruList;
		std::unordered_map<uint64_t, std::list<uint64_t>::iterator> _segment2LruMap;

		uint32_t _writeBufferSize;
		RingBuffer<uint64_t> *_writeBuffer;
		uint32_t _numWriteThread;
		std::vector<thread> _writeThreads;

		uint32_t _prefetchBufferSize;
		RingBuffer<std::pair<uint64_t, uint32_t> > *_prefetchBuffer;
		uint32_t _numPrefetchThread;
		std::vector<thread> _prefetchThreads;
		std::unordered_map<uint64_t, bool> _prefetchBitmap;
		std::mutex _prefetchBitmapMutex;
};
#endif 
