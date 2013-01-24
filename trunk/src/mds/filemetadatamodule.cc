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

	_collection = "FileMetaData";

	_fileMetaDataStorage = new MongoDB();
	_fileMetaDataStorage->connect();
	_fileMetaDataStorage->setCollection(_collection);

	//BSONObj querySegment = BSON ("id" << "config");
	//BSONObj updateSegment = BSON ("$set" << BSON ("fileId" << 0));
	//_fileMetaDataStorage->update(querySegment, updateSegment);

}

/**
 * @brief	Create a File
 */
void FileMetaDataModule::createFile(uint32_t clientId, const string &path,
		uint64_t fileSize, uint32_t fileId) {
	BSONObj insertSegment =
			BSON ("id" << fileId << "path" << path << "fileSize" << (long long int)fileSize
					<< "clientId" << clientId);
	_fileMetaDataStorage->insert(insertSegment);

	return;
}

/**
 * @brief	Delete a File
 */
void FileMetaDataModule::deleteFile(uint32_t fileId) {
	BSONObj querySegment = BSON ("id" << fileId);
	_fileMetaDataStorage->remove(querySegment);
}

/**
 * @brief	Rename a File
 */
void FileMetaDataModule::renameFile(uint32_t fileId, const string& newPath) {
	BSONObj querySegment = BSON ("id" << fileId);
	BSONObj updateSegment = BSON ("$set" << BSON ("path" << newPath));
	_fileMetaDataStorage->update(querySegment, updateSegment);
}

/**
 * @brief	Lookup the File ID with file Path
 */
uint32_t FileMetaDataModule::lookupFileId(const string &path)
{
	BSONObj querySegment = BSON ("path" << path);
	uint32_t fileId = 0;
	try {
		BSONObj result = _fileMetaDataStorage->readOne(querySegment);
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
	BSONObj querySegment = BSON ("id" << fileId);
	BSONObj updateSegment = BSON ("$set" << BSON ("fileSize" << (long long int)fileSize));
	_fileMetaDataStorage->update(querySegment, updateSegment);
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
	BSONObj querySegment = BSON ("id" << fileId);
	BSONObj result = _fileMetaDataStorage->readOne(querySegment);
	return (uint64_t)result.getField("fileSize").numberLong();
}

/**
 * @brief	Save the Segment List of a File
 */
void FileMetaDataModule::saveSegmentList(uint32_t fileId, const vector<uint64_t> &segmentList) {
	vector<uint64_t>::const_iterator it;
	BSONObj querySegment = BSON ("id" << fileId);
	BSONArrayBuilder arrb;
	for(it = segmentList.begin(); it < segmentList.end(); ++it) {
		arrb.append((long long int) *it);
	}
	BSONArray arr = arrb.arr();
	BSONObj updateSegment = BSON ("$set" << BSON ("segmentList" << arr));
	_fileMetaDataStorage->update(querySegment,updateSegment);
	return;
}

/**
 * @brief	Read the Segment List of a File
 */
vector<uint64_t> FileMetaDataModule::readSegmentList(uint32_t fileId) {
	vector<uint64_t> segmentList;
	BSONObj querySegment = BSON ("id" << fileId);
	BSONObj result = _fileMetaDataStorage->readOne(querySegment);
	BSONForEach(it, result.getObjectField("segmentList")) {
		segmentList.push_back((uint64_t)it.numberLong());
		debug("SegmentList %lld\n",it.numberLong());
	}
	return segmentList;
}

/**
 * @brief	Generate a New File ID
 */
uint32_t FileMetaDataModule::generateFileId() {

	return _configMetaDataModule->getAndInc("fileId");
}
