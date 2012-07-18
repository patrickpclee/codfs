#ifndef __NAMESPACE_MODULE_HH__
#define __NAMESPACE_MODULE_HH__

#include "../common/metadata.hh"

#include <stdint.h>
#include <string>
#include <vector>

class NameSpaceModule {
public:
	uint32_t createFile (uint32_t clientId, string path);
//	uint32_t deleteFile (string path, uint32_t clientId);
	uint32_t openFile (uint32_t clientId, string path);

	/**
	 * @brief	List Folder
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	Path to the Folder
	 */
	vector<FileMetaData> listFolder (uint32_t clientId, string path);
private:
	uint32_t newFileId (string path);
};
#endif
