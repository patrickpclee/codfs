#ifndef __METADATA_MODULE_HH__
#define __METADATA_MODULE_HH__

#include <stdint.h>
#include <vector>
using namespace std;

#include "configmetadatamodule.hh"
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
	 * @param	fileSize	Size of the File
	 * @param 	codingScheme	Coding Scheme of the file
	 * @param	codingSetting	Coding Scheme Setting
	 *
	 * @return	File ID
	 */
	uint32_t createFile(uint32_t clientId, const string &path, uint64_t fileSize,
			CodingScheme codingScheme, const string &codingSetting);

	/**
	 * @brief	Open a File
	 *
	 * @param	clientId	ID of the Client
	 * @param	fileId	ID of the File
	 */
	void openFile(uint32_t clientId, uint32_t filieId);

	/**
	 * @brief	Delete a File
	 *
	 * @param	clientId	ID of the Client
	 * @param	fileId	File ID
	 */
	void deleteFile(uint32_t clientId, uint32_t fileId);

	/**
	 *	@brief	Set File Size of a File
	 *
	 *	@param	fileId	ID of the File
	 *	@param	fileSize	Size of the File
	 */
	void setFileSize(uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief	Read File Size of a File
	 *
	 * @param	fileId	ID of the File
	 *
	 * @return	File Size
	 */
	uint64_t readFileSize(uint32_t fileId);

	/**
	 * @brief	Generate List of Object ID
	 *
	 * @param	numOfObjs	Number of Objects
	 *
	 * @return	List of Object ID
	 */
	vector<uint64_t> newObjectList(uint32_t numOfObjs);

	/**
	 * @brief	Save Object List of a File
	 *
	 * @param	fileId	ID of the File
	 * @param	objectList	List of Object ID
	 */
	void saveObjectList(uint32_t fileId, const vector<uint64_t> &objectList);
	vector<uint64_t> readObjectList(uint32_t fileId);
	vector<uint64_t> readOsdObjectList(uint32_t osdId);

	/**
	 * @brief	Read Checksum of a File
	 *
	 * @param	fileId	ID fo the file
	 *
	 * @return	Checksum
	 */
	string readChecksum(uint32_t fileId);

	/**
	 * @brief	Save Object Info
	 *
	 * @param	objectId	ID of the Object
	 * @param	objectInfo	Info of the Object
	 */
	void saveObjectInfo(uint64_t objectId, struct ObjectMetaData objectInfo);
	
	/**
	 * @brief	Read Object Info
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	Info of the Object
	 */
	struct ObjectMetaData readObjectInfo(uint64_t objectId);

	/**
	 * @brief	Set Primary of a Object
	 *
	 * @param	objectId	ID of the Object
	 * @param	primaryOsdId	ID of the Primary
	 */
	void setPrimary(uint64_t objectId, uint32_t primaryOsdId);

	uint32_t selectActingPrimary(uint64_t objectId, vector<uint32_t> nodeList,
		vector<bool> nodeStatus);

	/**
	 * @brief	Get Primary of a Object
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	ID of the Primary
	 */
	uint32_t getPrimary(uint64_t objectId);

	/**
	 * @brief	Save Node List of a Object
	 *
	 * @param	objectId	ID of the Object
	 * @param	objectNodeList	List of Node ID
	 */
	void saveNodeList(uint64_t objectId, const vector<uint32_t> &objectNodeList);

	/**
	 * @brief	Read Node List of a Object
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	List of Node ID
	 */
	vector<uint32_t> readNodeList(uint64_t objectId);

	/**
	 * @brief	Lookup the File Path with File ID
	 *
	 * @param	fileId	ID of the File
	 * 
	 * @return	Path to the File
	 */
	string lookupFilePath(uint32_t fileId);

	/**
	 * @brief	Lookup the File ID with file Path
	 *
	 * @param	path	Path to the File
	 *
	 * @return	ID of the File
	 */
	uint32_t lookupFileId(const string &path);

private:

	ConfigMetaDataModule* _configMetaDataStorage;
	FileMetaDataModule* _fileMetaDataModule;
	ObjectMetaDataModule* _objectMetaDataModule;
	OsdMetaDataModule* _osdMetaDataModule;
};

#endif
