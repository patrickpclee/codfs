#ifndef __FILE_METADATA_HH__
#define __FILE_METADATA_HH__

#include <stdint.h>

class FileMetaData {
public:
	uint32_t readFileMetaData (uint32_t fileId);
//	uint32_t writeFileMetaData (uint32_t fileId, ObjectMetaData objectMetaData);
	uint32_t createFileMetaData (uint64_t fileId);
	uint32_t deleteFileMetaData (uint64_t fileId);
private:
};
#endif
