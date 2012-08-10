#include "filemetadatamodule.hh"

#include "../config/config.hh"

#include "../common/debug.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

FileMetaDataModule::FileMetaDataModule()
{
	// TODO: Contact MDS other than MDS0

	_collection = "File Meta Data";

	_fileMetaDataStorage = new MongoDB();
	_fileMetaDataStorage->connect();

	//BSONObj queryObject = BSON ("id" << "config");
	//BSONObj updateObject = BSON ("$set" << BSON ("fileId" << 0));
	//_fileMetaDataStorage->update(_collection, queryObject, updateObject);
	
}

void FileMetaDataModule::createFile (uint32_t clientId, string path, uint32_t fileId)
{
	BSONObj insertObject = BSON ("id" << fileId << "path" << path << "clientId" << clientId);
	_fileMetaDataStorage->insert(_collection, insertObject);

	return;
}

void FileMetaDataModule::saveObjectList (uint32_t fileId, vector<uint64_t> objectList)
{

	return ;
}

uint32_t FileMetaDataModule::generateFileId()
{
	uint32_t fileId;

	BSONObj queryObject = BSON ("id" << "config");
	BSONObj updateObject = BSON ("$inc" << BSON ("fileId" << 1));
	BSONObj result = _fileMetaDataStorage->findAndModify("Configuration",queryObject,updateObject);
	
	fileId = result.getField("fileId").Int();

	return fileId;
}

FileMetaDataModule::~FileMetaDataModule()
{
}
