#ifndef __METADATA_MODULE_HH__
#define __METADATA_MODULE_HH__

#include <stdint.h>

#include "filemetadatamodule.hh"
#include "objectmetadatamodule.hh"

#include "../common/metadata.hh"

class MetaDataModule {
public:
	//Lookup Object ID List from File ID
	uint64_t* getFileObjectList (uint32_t fileId);
	// Lookup Object ID List from OSD ID
	uint64_t* getOsdObjectList (uint32_t osdId);

	// Select Acting Primary in case of Primary Failure
	uint32_t selectActingPrimary (uint64_t objectId);

	uint32_t createFileMetaData (uint32_t fileId, string path);
	uint32_t saveFileMetaData (FileMetaData	fileMetaData);
	uint32_t deleteFileMetaData (uint32_t fileId);
	uint32_t saveObjectList (uint32_t fileId, uint64_t objectIdList[]);

	uint64_t newObjectId ();
	uint32_t saveObjectMetaData (ObjectMetaData	objectMetaData);
	uint32_t deleteObjectMetaData (uint64_t objectId);
	uint32_t saveNodeList (uint64_t objectId, uint32_t osdIdList[]);

private:
	FileMetaDataModule _fileMetaDataModule;
	ObjectMetaDataModule _objectMetaDataModule;
};

#endif
