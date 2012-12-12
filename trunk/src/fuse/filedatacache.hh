#ifndef __FILE_DATA_CACHE_HH__
#define __FILE_DATA_CACHE_HH__

//#include <boost/thread/locks.hpp>
//#include <boost/thread/shared_mutex.hpp>

//#include "../cache/lru_cache.hh"

#include "../common/metadata.hh"
#include "../common/enums.hh"
#include "../common/segmentdata.hh"

#include <mutex>

//#include <memory>

class FileDataCache {
	public:
		FileDataCache (struct FileMetaData fileMetaData, uint64_t segmentSize);

		//int64_t read(void* buf, uint32_t size, uint64_t offset);
		int64_t write(const void* buf, uint32_t size, uint64_t offset);

		~FileDataCache ();
		void writeBack(uint32_t index);
		void flush();
	private:
		bool _clean;
		uint64_t _segmentSize;
		uint32_t _lastSegmentCount;
		uint64_t _fileSize;
		uint32_t _fileId;
		uint32_t _lastWriteBackPos;
		struct FileMetaData _metaData;
		vector<struct SegmentData> _segmentDataList;
		vector<SegmentDataStatus> _segmentStatusList;
		vector<uint32_t> _primaryList;
		mutex _writeBackMutex;
		//LruCache<uint64_t, shared_ptr<SegmentData> >* _segmentCache;
};

#endif
