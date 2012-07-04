#ifndef __FILE_METADATA_HH__
#define __FILE_METADATA_HH__

#include <stdint.h>

class FileMetaData {
public:
	uint32_t readFileMetaData (uint64_t objectId);
//	uint32_t writeFileMetaData (uint64_t objectId, ObjectMetaData objectMetaData);
	uint32_t createFileMetaData (uint64_t objectId);
	uint32_t deleteFileMetaData (uint64_t objectId);
private:
};
#endif
