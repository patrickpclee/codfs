#ifndef __NAMESPACE_MODULE_HH__
#define __NAMESPACE_MODULE_HH__

#include "../common/metadata.hh"

#include <stdint.h>
#include <string>

class NameSpaceModule {
public:
	uint32_t createFile (uint32_t clientId, string path);
//	uint32_t deleteFile (string path, uint32_t clientId);
	uint32_t openFile (uint32_t clientId, string path);
//	FileMetaData* listFolder (string path);
private:
	uint32_t newFileId (string path);
};
#endif
