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
void FileMetaDataModule::createFile(uint32_t clientId, string path,
		uint64_t fileSize, uint32_t fileId, CodingScheme codingScheme,
		string codingSetting) {
	BSONObj insertObject =
			BSON ("id" << fileId << "path" << path << "fileSize" << (long long int)fileSize
					<< "clientId" << clientId << "codingScheme" << (int)codingScheme
					<< "codingSetting" << codingSetting);
	_fileMetaDataStorage->insert(insertObject);

	return;
}

/**
 * @brief	Lookup the File ID with file Path
 */
uint32_t FileMetaDataModule::lookupFileId(string path)
{
	BSONObj queryObject = BSON ("path" << path.erase(0,1));
	BSONObj result = _fileMetaDataStorage->readOne(queryObject);
	return (uint32_t)result.getField("id").numberInt();
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
void FileMetaDataModule::saveObjectList(uint32_t fileId,
		vector<uint64_t> objectList) {
	vector<uint64_t>::iterator it;
	BSONObj queryObject = BSON ("id" << fileId);
	_fileMetaDataStorage->removeField(queryObject,"objectList");
	BSONObj pushObject;
	for (it = objectList.begin(); it < objectList.end(); ++it) {
		//arr << *it;
		pushObject =
				BSON ( "$push" << BSON ("objectList" << (long long int)*it));
		//debug("Push %" PRIu64 "\n", *it);
		_fileMetaDataStorage->push(queryObject, pushObject);
	}

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
