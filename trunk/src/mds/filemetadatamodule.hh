#ifndef __FILE_METADATA_MODULE_HH__
#define __FILE_METADATA_MODULE_HH__

#include <atomic>
#include <stdint.h>

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class FileMetaDataModule {
public:
	FileMetaDataModule();
	~FileMetaDataModule();

	void createFile (uint32_t clientId, string path, uint64_t fileSize, uint32_t fileId, CodingScheme codingScheme);
	void saveObjectList (uint32_t fileId, vector<uint64_t> objectList);
	vector<uint64_t> readObjectList(uint32_t fileId);
	void createFile(uint32_t clientId, string path, uint64_t fileSize,
			uint32_t fileId, CodingScheme codingScheme, string codingSetting);

	uint32_t generateFileId();
private:
	string _collection;

	MongoDB* _fileMetaDataStorage;
	/// File Meta Data Cache
	//FileMetaDataCache* _fileMetaDataCache;

};
#endif
