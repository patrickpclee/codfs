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
	MetaDataModule();
	
	/**
	 * @brief	Create Meta Data Entry for a New File
	 *
	 * @param	cleintId	ID of the Client
	 * @param	path	Path to the File
	 *
	 * @return	File ID
	 */
	uint32_t createFile (uint32_t clientId, string path);

	/**
	 * @brief	Open a File
	 *
	 * @param	clientId	ID of the Client
	 * @param	fileId	ID of the File
	 */
	void openFile (uint32_t clientId, uint32_t filieId);

	uint32_t selectActingPrimary (uint64_t objectId, uint32_t exclude);

	uint32_t saveObjectList (uint32_t fileId, vector<uint64_t> objectList);
	vector<uint64_t> readObjectList (uint32_t fileId);
	vector<uint64_t> readOsdObjectList (uint32_t osdId);

	/**
	 * @brief	Read Checksum of a File
	 *
	 * @param	fileId	ID fo the file
	 *
	 * @return	Checksum
	 */
	unsigned char* readChecksum (uint32_t fileId);

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

	/**
	 * @brief	Lookup the File Path with File ID
	 *
	 * @param	fileId	ID of the File
	 * 
	 * @return	Path to the File
	 */
	string lookupFilePath (uint32_t fileId);

	/**
	 * @brief	Lookup the File ID with file Path
	 *
	 * @param	path	Path to the File
	 *
	 * @return	ID of the File
	 */
	uint32_t lookupFileId (string path);

private:
	/**
	 * @brief	Generate a New ObjectId
	 *
	 * @return	Object ID
	 */
	uint64_t newObjectId ();

	FileMetaDataModule* _fileMetaDataModule;
	ObjectMetaDataModule* _objectMetaDataModule;
	OsdMetaDataModule* _osdMetaDataModule;
};

#endif
