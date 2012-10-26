#include "metadatamodule.hh"

#include "../common/debug.hh"

MetaDataModule::MetaDataModule() {
	_configMetaDataStorage = new ConfigMetaDataModule();
	_fileMetaDataModule = new FileMetaDataModule(_configMetaDataStorage);
	_objectMetaDataModule = new ObjectMetaDataModule(_configMetaDataStorage);
	_osdMetaDataModule = new OsdMetaDataModule();

	srand(time(NULL));
}

/**
 * @brief	Create Meta Data Entry for a New File
 */
uint32_t MetaDataModule::createFile(uint32_t clientId, const string &path,
		uint64_t fileSize, CodingScheme codingScheme,
		const string &codingSetting) {
	uint32_t fileId = _fileMetaDataModule->generateFileId();

	_fileMetaDataModule->createFile(clientId, path, fileSize, fileId,
			codingScheme, codingSetting);

	return fileId;
}

/**
 * @brief	Open a File
 */
void MetaDataModule::openFile(uint32_t clientId, uint32_t fileId) {
	return;
}

void MetaDataModule::deleteFile(uint32_t clientId, uint32_t fileId) {
	_fileMetaDataModule->deleteFile(fileId);
}

/**
 *	@brief	Set File Size of a File
 */
void MetaDataModule::setFileSize(uint32_t fileId, uint64_t fileSize) {
	_fileMetaDataModule->setFileSize(fileId, fileSize);
	return;
}

/**
 * @brief	Read File Size of a File
 */
uint64_t MetaDataModule::readFileSize(uint32_t fileId) {
	return _fileMetaDataModule->readFileSize(fileId);
}

/**
 * @brief	Generate List of Object ID
 */
vector<uint64_t> MetaDataModule::newObjectList(uint32_t numOfObjs) {
	vector<uint64_t> objectList(numOfObjs);
	for (uint32_t i = 0; i < numOfObjs; ++i) {
		//objectList.push_back(newObjectId());
		objectList[i] = _objectMetaDataModule->generateObjectId();
	}
	return objectList;
}

/**
 * @brief	Save Object List of a File
 */
void MetaDataModule::saveObjectList(uint32_t fileId,
		const vector<uint64_t> &objectList) {
	debug("Save %d %zu\n", fileId, objectList.size());
	_fileMetaDataModule->saveObjectList(fileId, objectList);

	return;
}

vector<uint64_t> MetaDataModule::readObjectList(uint32_t fileId) {
	return _fileMetaDataModule->readObjectList(fileId);
}

vector<uint64_t> MetaDataModule::readOsdObjectList(uint32_t osdId) {
	return {0};
}

string MetaDataModule::readChecksum(uint32_t fileId) {
	return ""; // null
}

/**
 * @brief	Save Object Info
 */
void MetaDataModule::saveObjectInfo(uint64_t objectId,
		struct ObjectMetaData objectInfo) {
	_objectMetaDataModule->saveObjectInfo(objectId, objectInfo);

	return;
}

/**
 * @brief	Read Object Info
 *
 * @param	objectId	ID of the Object
 *
 * @return	Info of the Object
 */
struct ObjectMetaData MetaDataModule::readObjectInfo(uint64_t objectId) {
	return _objectMetaDataModule->readObjectInfo(objectId);
}

/**
 * @brief	Set Primary of a Object
 */
void MetaDataModule::setPrimary(uint64_t objectId, uint32_t primaryOsdId) {
	_objectMetaDataModule->setPrimary(objectId, primaryOsdId);
	return;
}

uint32_t MetaDataModule::selectActingPrimary(uint64_t objectId, vector<uint32_t> nodeList,
		vector<bool> nodeStatus) {

	int failedOsdCount = (int) count(nodeStatus.begin(),
			nodeStatus.end(), false);

	if (failedOsdCount == nodeStatus.size()) {
		debug_yellow ("All OSD has died for objectId = %" PRIu64 "\n", objectId);
		exit (-1);
	}

	srand(time(NULL));

	// random until a healthy OSD is found
	while (true) {
		int newPrimary = rand() % nodeStatus.size();
		if (nodeStatus[newPrimary] == true) {
			setPrimary (objectId, newPrimary);
			return newPrimary;
		}
	}

	return 0;
}

/**
 * @brief	Get Primary of a Object
 */
uint32_t MetaDataModule::getPrimary(uint64_t objectId) {
	return _objectMetaDataModule->getPrimary(objectId);
}

/**
 * @brief	Save Node List of a Object
 */
void MetaDataModule::saveNodeList(uint64_t objectId,
		const vector<uint32_t> &objectNodeList) {
	_objectMetaDataModule->saveNodeList(objectId, objectNodeList);
	return;
}

/**
 * @brief	Read Node List of a Object
 */
vector<uint32_t> MetaDataModule::readNodeList(uint64_t objectId) {
	return _objectMetaDataModule->readNodeList(objectId);
}

uint32_t MetaDataModule::lookupFileId(const string &path) {
	return _fileMetaDataModule->lookupFileId(path);
}

string MetaDataModule::lookupFilePath(uint32_t fileId) {
	return "";
}
