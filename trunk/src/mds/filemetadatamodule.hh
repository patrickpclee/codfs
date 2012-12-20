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
	 */
	void createFile(uint32_t clientId, const string &path, uint64_t fileSize,
			uint32_t fileId);

	/**
	 * @brief	Delete a File
	 *
	 * @param	fileId	File Id
	 */
	void deleteFile(uint32_t fileId);

	/**
	 * @brief	Rename a File
	 *
	 * @param	fileId	File ID
	 * @param	newPath	New File Path
	 */
	void renameFile(uint32_t fileId, const string& newPath);

	/**
	 * @brief	Lookup the File ID with file Path
	 *
	 * @param	path	Path to the File
	 *
	 * @return	ID of the File
	 */
	uint32_t lookupFileId(const string &path);

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
	 * @brief	Save the Segment List of a File
	 *
	 * @param	fileId	ID of the File
	 * @param	segmentList	List of Segment ID
	 */
	void saveSegmentList (uint32_t fileId, const vector<uint64_t> &segmentList);

	/**
	 * @brief	Read the Segment List of a File
	 *
	 * @param	fileId ID of the File
	 *
	 * @raturn	List of Segment ID
	 */
	vector<uint64_t> readSegmentList(uint32_t fileId);

	/**
	 * @brief	Generate a New File ID
	 *
	 * @return	File ID
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
