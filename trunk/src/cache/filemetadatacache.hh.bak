#ifndef __FILE_METADATA_CACHE_HH__
#define __FILE_METADATA_CACHE_HH__

#include "cache.hh"
#include "mm/cache_map.hpp"
#include "../common/metadata.hh"

#include <stdint.h>

class FileMetaDataCache : public Cache {
public:
	FileMetaDataCache();

	~FileMetaDataCache();

	FileMetaData* readFileMetaData (uint32_t fileId);

	void writeFileMetaData (uint32_t fileId, FileMetaData * fileMetaData);

	//void createFileMetaData (uint32_t fileId);

	void deleteFileMetaData (uint32_t fileId);

	char* read (uint64_t id);

	void write (uint64_t id, char* data);

	void deleteEntry (uint64_t id);
private:
	mm::cache_map<uint32_t, FileMetaData*> _fileMetaDataMap;
};
#endif
