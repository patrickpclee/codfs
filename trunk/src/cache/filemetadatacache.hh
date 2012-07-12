#ifndef __FILE_METADATA_CACHE_HH__
#define __FILE_METADATA_CACHE_HH__

#include "../cache/cache.hh"
#include "../common/metadata.hh"

#include <stdint.h>

class FileMetaDataCache : public Cache {
public:
	uint32_t readFileMetaData (uint32_t fileId);
	uint32_t writeFileMetaData (uint32_t fileId, FileMetaData fileMetaData);
	uint32_t createFileMetaData (uint32_t fileId);
	uint32_t deleteFileMetaData (uint32_t fileId);
private:
};
#endif
