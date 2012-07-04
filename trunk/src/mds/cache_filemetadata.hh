#ifndef __CACHE_FILE_METADATA_HH__
#define __CACHE_FILE_METADATA_HH__

#include "../cache/cache.hh"

#include <stdint.h>

class FileMetaDataCache : public Cache {
public:
	uint32_t readFileMetaData (uint64_t objectId);
//	uint32_t writeFileMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createFileMetaData (uint64_t objectId);
	uint32_t deleteFileMetaData (uint64_t objectId);
private:
};
#endif
