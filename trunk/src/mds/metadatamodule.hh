#ifndef __METADATA_MODULE_HH__
#define __METADATA_MODULE_HH__

#include <stdint.h>
#include <vector>
using namespace std;

#include "filemetadatamodule.hh"
#include "objectmetadatamodule.hh"

#include "../common/metadata.hh"

class MetaDataModule {
public:
	//Lookup Object ID List from File ID
//	uint64_t* getFileObjectList (uint32_t fileId);
	// Lookup Object ID List from OSD ID
//	uint64_t* getOsdObjectList (uint32_t osdId);o

	string lookupFilePath (uint32_t fileId);
	uint32_t lookupFileId (string path);

	// Select Acting Primary in case of Primary Failure
//	uint32_t selectActingPrimary (uint64_t objectId);

	void openFile (uint32_t clientId, uint32_t filieId);
	uint32_t createFile (string path);
	uint32_t saveObjectList (uint32_t fileId, vector<uint64_t> objectList);
	vector<uint64_t> readObjectList (uint32_t fileId);

//	uint32_t saveFileMetaData (FileMetaData	fileMetaData);
//	uint32_t deleteFileMetaData (uint32_t fileId);
//	uint32_t saveObjectList (uint32_t fileId, uint64_t objectIdList[]);

	uint64_t newObjectId ();
//	uint32_t saveObjectMetaData (ObjectMetaData	objectMetaData);
//	uint32_t deleteObjectMetaData (uint64_t objectId);

	void setPrimary (uint64_t objectId, uint32_t primaryOsdId);
	uint32_t getPrimary (uint64_t objectId);

	void saveNodeList (uint64_t objectId, vector<uint32_t> osdIdList);
	vector<uint32_t> readNodeList (uint64_t objectId);


private:
	FileMetaDataModule _fileMetaDataModule;
	ObjectMetaDataModule _objectMetaDataModule;
};

#endif
