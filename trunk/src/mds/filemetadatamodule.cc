#include "filemetadatamodule.hh"

#include "../config/config.hh"

#include "../common/debug.hh"

extern ConfigLayer *configLayer;

FileMetaDataModule::FileMetaDataModule()
{
	// TODO: Contact MDS other than MDS0

	_collection = "File Meta Data";

	_fileMetaDataStorage = new MongoDB();
	_fileMetaDataStorage->connect();
	
	_nextFileId = 0;
	mongo::BSONObj temp = _fileMetaDataStorage->read(_collection, BSON("id" << "config")).at(0);

	debug("%s\n","Get Max File ID");
	_nextFileId = temp.getField("nextFileId").Int();

}

uint32_t FileMetaDataModule::generateFileId()
{
	_nextFileId++;
	uint32_t fileId = _nextFileId;
	debug("%d\n",fileId);
	_fileMetaDataStorage->update(_collection, BSON("id" << "config"), BSON("id" << "config" << "nextFileId" << fileId));
	return fileId - 1;
}

FileMetaDataModule::~FileMetaDataModule()
{
	//_fileMetaDataStorage->update(_collection, BSON("id" << "config"), BSON("nextFileId" << _nextFileId));
}
