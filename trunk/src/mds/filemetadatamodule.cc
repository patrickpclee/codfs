#include "filemetadatamodule.hh"

#include "../config/config.hh"

#include "../common/debug.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

/**
 * @brief	Default Constructor
 */
FileMetaDataModule::FileMetaDataModule(ConfigMetaDataModule* configMetaDataModule) {
	_configMetaDataModule = configMetaDataModule;

	_collection = "File Meta Data";

	_fileMetaDataStorage = new MongoDB();
	_fileMetaDataStorage->connect();
	_fileMetaDataStorage->setCollection(_collection);

	//BSONObj queryObject = BSON ("id" << "config");
	//BSONObj updateObject = BSON ("$set" << BSON ("fileId" << 0));
	//_fileMetaDataStorage->update(queryObject, updateObject);

}

/**
 * @brief	Create a File
 */
void FileMetaDataModule::createFile(uint32_t clientId, const string &path,
		uint64_t fileSize, uint32_t fileId, CodingScheme codingScheme,
		const string &codingSetting) {
	BSONObj insertObject =
			BSON ("id" << fileId << "path" << path << "fileSize" << (long long int)fileSize
					<< "clientId" << clientId << "codingScheme" << (int)codingScheme
					<< "codingSetting" << codingSetting);
	_fileMetaDataStorage->insert(insertObject);

	return;
}

/**
 * @brief	Delete a File
 */
void FileMetaDataModule::deleteFile(uint32_t fileId) {
	BSONObj queryObject = BSON ("id" << fileId);
	_fileMetaDataStorage->remove(queryObject);
}

/**
 * @brief	Rename a File
 */
void FileMetaDataModule::renameFile(uint32_t fileId, const string& newPath) {
	BSONObj queryObject = BSON ("id" << fileId);
	BSONObj updateObject = BSON ("$set" << BSON ("path" << newPath));
	_fileMetaDataStorage->update(queryObject, updateObject);
}

/**
 * @brief	Lookup the File ID with file Path
 */
uint32_t FileMetaDataModule::lookupFileId(const string &path)
{
	BSONObj queryObject = BSON ("path" << path);
	uint32_t fileId = 0;
	try {
		BSONObj result = _fileMetaDataStorage->readOne(queryObject);
		fileId = (uint32_t)result.getField("id").numberInt();
	} catch (...) {
	}
	return fileId;
}

/**
 *	@brief	Set File Size of a File
 *
 *	@param	fileId	ID of the File
 *	@param	fileSize	Size of the File
 */
void FileMetaDataModule::setFileSize(uint32_t fileId, uint64_t fileSize)
{
	BSONObj queryObject = BSON ("id" << fileId);
	BSONObj updateObject = BSON ("$set" << BSON ("fileSize" << (long long int)fileSize));
	_fileMetaDataStorage->update(queryObject, updateObject);
	return ;
}

/**
 * @brief	Read File Size of a File
 *
 * @param	fileId	ID of the File
 *
 * @return	File Size
 */
uint64_t FileMetaDataModule::readFileSize(uint32_t fileId)
{
	BSONObj queryObject = BSON ("id" << fileId);
	BSONObj result = _fileMetaDataStorage->readOne(queryObject);
	return (uint64_t)result.getField("fileSize").numberLong();
}

/**
 * @brief	Save the Object List of a File
 */
void FileMetaDataModule::saveObjectList(uint32_t fileId, const vector<uint64_t> &objectList) {
	vector<uint64_t>::const_iterator it;
	BSONObj queryObject = BSON ("id" << fileId);
	BSONArrayBuilder arrb;
	for(it = objectList.begin(); it < objectList.end(); ++it) {
		arrb.append((long long int) *it);
	}
	BSONArray arr = arrb.arr();
	BSONObj updateObject = BSON ("$set" << BSON ("objectList" << arr));
	_fileMetaDataStorage->update(queryObject,updateObject);
	return;
}

/**
 * @brief	Read the Object List of a File
 */
vector<uint64_t> FileMetaDataModule::readObjectList(uint32_t fileId) {
	vector<uint64_t> objectList;
	BSONObj queryObject = BSON ("id" << fileId);
	BSONObj result = _fileMetaDataStorage->readOne(queryObject);
	BSONForEach(it, result.getObjectField("objectList")) {
		objectList.push_back((uint64_t)it.numberLong());
		debug("ObjectList %lld\n",it.numberLong());
	}
	return objectList;
}

/**
 * @brief	Generate a New File ID
 */
uint32_t FileMetaDataModule::generateFileId() {

	return _configMetaDataModule->getAndInc("fileId");
}
