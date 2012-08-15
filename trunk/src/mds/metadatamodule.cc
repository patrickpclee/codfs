#include "metadatamodule.hh"

#include "../common/debug.hh"

MetaDataModule::MetaDataModule() {
	_fileMetaDataModule = new FileMetaDataModule();
	_objectMetaDataModule = new ObjectMetaDataModule();
	_osdMetaDataModule = new OsdMetaDataModule();

	srand(0);
}

string MetaDataModule::lookupFilePath(uint32_t fileId) {
	return "";
}

uint32_t MetaDataModule::createFile(uint32_t clientId, string path,
		uint64_t fileSize, CodingScheme codingScheme) {
	uint32_t fileId = _fileMetaDataModule->generateFileId();

	_fileMetaDataModule->createFile(clientId, path, fileSize, fileId, codingScheme);

	return fileId;
}

uint32_t MetaDataModule::getPrimary(uint64_t objectId) {
	return _objectMetaDataModule->getPrimary(objectId);
}

uint32_t MetaDataModule::lookupFileId(string path) {
	return 0;
}

void MetaDataModule::saveObjectList(uint32_t fileId,
		vector<uint64_t> objectList) {
	debug("Save %d %zu\n", fileId, objectList.size());
	_fileMetaDataModule->saveObjectList(fileId, objectList);

	return;
}

uint32_t MetaDataModule::selectActingPrimary(uint64_t objectId,
		uint32_t exclude) {
	return 0;
}

unsigned char* MetaDataModule::readChecksum(uint32_t fileId) {
	return 0; // null
}

vector<uint32_t> MetaDataModule::readNodeList(uint64_t objectId) {
	//return _fileMetaDataModule->readNodeList(objectId);
	return{0};
}

vector<uint64_t> MetaDataModule::newObjectList(uint32_t numOfObjs) {
	vector<uint64_t> objectList(numOfObjs);
	for (uint32_t i = 0; i < numOfObjs; ++i) {
		//objectList.push_back(newObjectId());
		objectList[i] = newObjectId();
	}
	return objectList;
}

uint64_t MetaDataModule::newObjectId() {
	return rand();
}

vector<uint64_t> MetaDataModule::readObjectList(uint32_t fileId) {
	return _fileMetaDataModule->readObjectList(fileId);
}

vector<uint64_t> MetaDataModule::readOsdObjectList(uint32_t osdId) {
	return {0};
}

void MetaDataModule::openFile(uint32_t clientId, uint32_t filieId) {
	return;
}

void MetaDataModule::saveNodeList(uint64_t objectId,
		vector<uint32_t> objectNodeList) {
	_objectMetaDataModule->saveNodeList(objectId, objectNodeList);
	return;
}

void MetaDataModule::setPrimary(uint64_t objectId, uint32_t primaryOsdId) {
	_objectMetaDataModule->setPrimary(objectId, primaryOsdId);
	return;
}

