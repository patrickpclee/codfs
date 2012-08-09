#ifndef __FILE_METADATA_MODULE_HH__
#define __FILE_METADATA_MODULE_HH__

#include <atomic>
#include <stdint.h>

#include "../cache/filemetadatacache.hh"

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"


class FileMetaDataModule {
public:
	FileMetaDataModule();
	~FileMetaDataModule();
	
	uint32_t generateFileId();
private:
	atomic<uint32_t> _nextFileId;

	string _collection;
	string _database;

	MongoDB* _fileMetaDataStorage;
	/// File Meta Data Cache
	FileMetaDataCache* _fileMetaDataCache;

};
#endif
