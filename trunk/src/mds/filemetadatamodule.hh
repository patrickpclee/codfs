#ifndef __FILE_METADATA_MODULE_HH__
#define __FILE_METADATA_MODULE_HH__

#include <stdint.h>

#include "filemetadatacache.hh"

#include "../common/metadata.hh"


class FileMetaDataModule {
public:
	uint32_t readFileMetaData (uint32_t fileId);
	uint32_t writeFileMetaData (uint32_t fileId, FileMetaData fileMetaData);
	uint32_t createFileMetaData (uint32_t fileId);
	uint32_t deleteFileMetaData (uint32_t fileId);
private:
	FileMetaDataCache* _fileMetaDataCache;
};
#endif
