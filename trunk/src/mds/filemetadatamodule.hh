#ifndef __FILE_METADATA_MODULE_HH__
#define __FILE_METADATA_MODULE_HH__

#include <stdint.h>

#include "configmetadatamodule.hh"

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class FileMetaDataModule {
public:
	/**
	 * @brief	Default Constructor
	 *
	 * @param	configMetaDataModule	Configuration Meta Data Module
	 */
	FileMetaDataModule(ConfigMetaDataModule* configMetaDataModule);

	/**
	 * @brief	Create a File
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	Path of the File
	 * @param	fileSize	Size of the File
	 * @param	codingScheme	Coding Scheme
	 * @param	codingSetting	Setting of Coding
	 */
	void createFile(uint32_t clientId, string path, uint64_t fileSize,
			uint32_t fileId, CodingScheme codingScheme, string codingSetting);
	void saveObjectList (uint32_t fileId, vector<uint64_t> objectList);
	vector<uint64_t> readObjectList(uint32_t fileId);

	/**
	 * @brief	Generate a New File ID
	 */
	uint32_t generateFileId();
private:
	/// Collection
	string _collection;

	/// Configuration Meta Data Module
	ConfigMetaDataModule* _configMetaDataModule;
	
	/// Underlying Meta Data Storage
	MongoDB* _fileMetaDataStorage;
	/// File Meta Data Cache
	//FileMetaDataCache* _fileMetaDataCache;

};
#endif
