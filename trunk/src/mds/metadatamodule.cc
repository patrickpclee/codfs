#include "metadatamodule.hh"

MetaDataModule::MetaDataModule()
{
	_fileMetaDataModule = new FileMetaDataModule();
	_objectMetaDataModule = new ObjectMetaDataModule();
	_osdMetaDataModule = new OsdMetaDataModule();
}

string MetaDataModule::lookupFilePath(uint32_t fileId)
{
	return "";
}


uint32_t MetaDataModule::createFile(uint32_t clientId, string path)
{	
	uint32_t fileId = _fileMetaDataModule->generateFileId();	

	return fileId;
}


uint32_t MetaDataModule::getPrimary(uint64_t objectId)
{
	return 0;
}


uint32_t MetaDataModule::lookupFileId(string path)
{
	return 0;
}


uint32_t MetaDataModule::saveObjectList(uint32_t fileId, vector<uint64_t> objectList)
{
	return 0;
}


uint32_t MetaDataModule::selectActingPrimary(uint64_t objectId, uint32_t exclude)
{
	return 0;
}


uint64_t MetaDataModule::newObjectId()
{
	return 0;
}


unsigned char* MetaDataModule::readChecksum(uint32_t fileId)
{
	return 0; // null
}


vector<uint32_t> MetaDataModule::readNodeList(uint64_t objectId)
{
	return {0};
}


vector<uint64_t> MetaDataModule::newObjectList(uint32_t numOfObjs)
{
	return {0};
}


vector<uint64_t> MetaDataModule::readObjectList(uint32_t fileId)
{
	return {0};
}


vector<uint64_t> MetaDataModule::readOsdObjectList(uint32_t osdId)
{
	return {0};
}


void MetaDataModule::openFile(uint32_t clientId, uint32_t filieId)
{
	return ;
}


void MetaDataModule::saveNodeList(uint64_t objectId, vector<uint32_t> objectNodeList)
{
	return ;
}


void MetaDataModule::setPrimary(uint64_t objectId, uint32_t primaryOsdId)
{
	return ;
}

