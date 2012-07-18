#ifndef __METADATA_MODULE_HH__
#define __METADATA_MODULE_HH__

#include <stdint.h>
#include <vector>
using namespace std;

#include "filemetadatamodule.hh"
#include "objectmetadatamodule.hh"
#include "osdmetadatamodule.hh"

#include "../common/metadata.hh"

class MetaDataModule {
public:
	string lookupFilePath (uint32_t fileId);
	uint32_t lookupFileId (string path);

	uint32_t selectActingPrimary (uint64_t objectId, uint32_t exclude);

	void openFile (uint32_t clientId, uint32_t filieId);
	uint32_t createFile (string path);
	uint32_t saveObjectList (uint32_t fileId, vector<uint64_t> objectList);
	vector<uint64_t> readObjectList (uint32_t fileId);
	vector<uint64_t> readOsdObjectList (uint32_t osdId);

	/**
	 * @brief	Generate List of Object ID
	 *
	 * @param	numOfObjs	Number of Objects
	 */
	vector<uint64_t> newObjectList (uint32_t numOfObjs);

	void setPrimary (uint64_t objectId, uint32_t primaryOsdId);
	uint32_t getPrimary (uint64_t objectId);

	void saveNodeList (uint64_t objectId, vector<uint32_t> objectNodeList);
	vector<uint32_t> readNodeList (uint64_t objectId);


private:
	uint64_t newObjectId ();

	FileMetaDataModule* _fileMetaDataModule;
	ObjectMetaDataModule* _objectMetaDataModule;
	OsdMetaDataModule* _osdMetaDataModule;
};

#endif
