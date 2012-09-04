#ifndef __FILE_DATA_CACHE_HH__
#define __FILE_DATA_CACHE_HH__

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "../common/metadata.hh"
#include "../common/enums.hh"
#include "../common/objectdata.hh"

class FileDataCache {
	public:
		FileDataCache (struct FileMetaData fileMetaData, uint64_t objectSize);

		//int64_t read(void* buf, uint32_t size, uint64_t offset);
		int64_t write(void* buf, uint32_t size, uint64_t offset);

		~FileDataCache ();
	private:
		uint64_t _objectSize;
		struct FileMetaData _metaData;
		vector<struct ObjectData> _objectDataList;
		vector<ObjectDataStatus> _objectStatusList;
};

#endif
