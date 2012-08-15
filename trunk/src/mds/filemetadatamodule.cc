#include "filemetadatamodule.hh"

#include "../config/config.hh"

#include "../common/debug.hh"

extern ConfigLayer *configLayer;

using namespace mongo;

FileMetaDataModule::FileMetaDataModule() {
	_collection = "File Meta Data";

	_fileMetaDataStorage = new MongoDB();
	_fileMetaDataStorage->connect();

	//BSONObj queryObject = BSON ("id" << "config");
	//BSONObj updateObject = BSON ("$set" << BSON ("fileId" << 0));
	//_fileMetaDataStorage->update(_collection, queryObject, updateObject);

}

void FileMetaDataModule::createFile(uint32_t clientId, string path,
		uint64_t fileSize, uint32_t fileId, CodingScheme codingScheme) {
	BSONObj insertObject =
			BSON ("id" << fileId << "path" << path << "fileSize" << (long long int)fileSize << "clientId" << clientId << "codingScheme" << (int)codingScheme);
	_fileMetaDataStorage->insert(_collection, insertObject);

	return;
}

void FileMetaDataModule::saveObjectList(uint32_t fileId,
		vector<uint64_t> objectList) {
	vector<uint64_t>::iterator it;
	BSONObj queryObject = BSON ("id" << fileId);
	BSONObj pushObject;
	for (it = objectList.begin(); it < objectList.end(); ++it) {
		//arr << *it;
		pushObject =
				BSON ( "$push" << BSON ("objectList" << (long long int)*it));
		debug("Push %" PRIu64 "\n", *it);
		_fileMetaDataStorage->push(_collection, queryObject, pushObject);
	}

	return;
}

uint32_t FileMetaDataModule::generateFileId() {
	uint32_t fileId;

	BSONObj queryObject = BSON ("id" << "config");
	BSONObj updateObject = BSON ("$inc" << BSON ("fileId" << 1));
	BSONObj result = _fileMetaDataStorage->findAndModify("Configuration",
			queryObject, updateObject);

	fileId = result.getField("fileId").Int();

	return fileId;
}

FileMetaDataModule::~FileMetaDataModule() {
}
