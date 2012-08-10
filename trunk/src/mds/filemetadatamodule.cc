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
	
}

uint32_t FileMetaDataModule::generateFileId()
{
	uint32_t fileId;

	mongo::BSONObj queryObject = BSON ("id" << "config");
	mongo::BSONObj updateObject = BSON ("$inc" << BSON ("fileId" << 1));
	mongo::BSONObj result = _fileMetaDataStorage->findAndModify(_collection,queryObject,updateObject);
	
	//debug("%s\n",result.jsonString().c_str());

	fileId = result.getField("fileId").Int();
//	debug("File ID = %d\n",fileId);

	return fileId;
}

FileMetaDataModule::~FileMetaDataModule()
{
	//_fileMetaDataStorage->update(_collection, BSON("id" << "config"), BSON("nextFileId" << _nextFileId));
}
