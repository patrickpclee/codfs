#ifndef __FILE_DATA_MODEULE_HH__
#define __FILE_DATA_MODEULE_HH__

#include <unordered_map>

#include <thread>
//#include <boost/thread/locks.hpp>
//#include <boost/thread/mutex.hpp>

#include "filedatacache.hh"

#include "../common/metadata.hh"
#include "../config/config.hh"

class FileDataModule {
	public:
		FileDataCache* createFileDataCache (struct FileMetaData fileMetaData);
		void closeFileDataCache(uint32_t fileId);
	private:
		uint64_t objectSize;

		std::mutex _fileDataCacheMapMutex;
		unordered_map <uint32_t, FileDataCache*> _fileDataCacheMap;

		//boost::shared_mutex _fileReferenceCountMapMutex;
		std::mutex _fileReferenceCountMapMutex;
		unordered_map <uint32_t, uint32_t> _fileReferenceCountMap;
};

#endif
