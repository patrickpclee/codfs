/**
 * mds.cc
 */

#include <cstdio>
#include <thread>

#include "mds.hh"

#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../config/config.hh"

using namespace std;

/**
 *	Global Variables
 */

/// MDS Object
Mds* mds;

/// Config Layer
ConfigLayer* configLayer;

/**
 * Initialise MDS Communicator and MetaData Modules
 */
Mds::Mds() {
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
	_mdsCommunicator = new MdsCommunicator();
}

Mds::~Mds() {
	delete _mdsCommunicator;
	delete _metaDataModule;
	delete _nameSpaceModule;
}

/**
 * @brief	Handle File Upload Request From Client
 *
 * 1. Create File in the Name Space (Directory Tree) \n
 * 2. Create File Meta Data (Generate File ID) \n
 * 3. Generate Object IDs \n
 * 4. Ask Monitor for Primary list \n
 * 5. Reply with Object and Primary List
 */
uint32_t Mds::uploadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &dstPath, uint64_t fileSize,
		uint32_t numOfObjs, CodingScheme codingScheme,
		const string &codingSetting) {

	vector<uint64_t> objectList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;

	_nameSpaceModule->createFile(clientId, dstPath);
	fileId = _metaDataModule->createFile(clientId, dstPath, fileSize,
			codingScheme, codingSetting);

	objectList = _metaDataModule->newObjectList(numOfObjs);
	_metaDataModule->saveObjectList(fileId, objectList);

//	primaryList = _mdsCommunicator->askPrimaryList(numOfObjs);
	primaryList = _mdsCommunicator->getPrimaryList(
			_mdsCommunicator->getMonitorSockfd(), numOfObjs);
	for (uint32_t i = 0; i < primaryList.size(); i++) {
		debug("Get primary list index %" PRIu32 " = %" PRIu32 "\n",
				i, primaryList[i]);
	}

	_mdsCommunicator->replyObjectandPrimaryList(requestId, connectionId, fileId,
			objectList, primaryList);

	return fileId;
}

void Mds::deleteFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path) {

	string tmpPath = path;
	if (fileId != 0)
		tmpPath = _metaDataModule->lookupFilePath(fileId);
	else
		fileId = _metaDataModule->lookupFileId(tmpPath);

	debug("Delete File %s [%" PRIu32 "]\n", tmpPath.c_str(), fileId);
	_nameSpaceModule->deleteFile(clientId, tmpPath);
	_metaDataModule->deleteFile(clientId, fileId);
	_mdsCommunicator->replyDeleteFile(requestId, connectionId, fileId);
}

/**
 * @brief	Handle Upload Object Acknowledgement from Primary
 *
 * 1. Save the Node List of the object \n
 * 2. Set the Primary for the object
 */
/*
 void Mds::FileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t fileId){
 uint64_t fileSize = _metaDataModule->readFileSize(fileId);
 _mdsCommunicator->replyFileSize(requestId, connectionId, fileId, fileSize);
 }
 */

void Mds::uploadObjectAckProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, uint32_t objectSize, CodingScheme codingScheme,
		const string &codingSetting, const vector<uint32_t> &objectNodeList,
		const string &checksum) {
	struct ObjectMetaData objectMetaData;
	objectMetaData._id = objectId;
	objectMetaData._nodeList = objectNodeList;
	objectMetaData._primary = objectNodeList[0];
	objectMetaData._codingScheme = codingScheme;
	objectMetaData._codingSetting = codingSetting;
	objectMetaData._checksum = checksum;
	objectMetaData._size = objectSize;
	_metaDataModule->saveObjectInfo(objectId, objectMetaData);
	//_metaDataModule->saveNodeList(objectId, objectNodeList);
	//_metaDataModule->setPrimary(objectId, objectNodeList[0]);

	return;
}

/**
 * @brief	Handle Download File Request from Client (Request with Path)
 */
void Mds::downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &dstPath) {
	uint32_t fileId = _metaDataModule->lookupFileId(dstPath);
	debug("Path = %s [%" PRIu32 "]\n", dstPath.c_str(), fileId);
	return downloadFileProcess(requestId, connectionId, clientId, fileId,
			dstPath);
}

/**
 * @brief	Handle Download File Request from Client (Request with File ID)
 */
void Mds::downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId) {
	string path = _metaDataModule->lookupFilePath(fileId);
	return downloadFileProcess(requestId, connectionId, clientId, fileId, path);
}

/**
 * @brief	Process the Download Request
 *
 * 1. Open File in the Name Space \n
 * 2. Open File Metadata \n
 * 3. Read Object List from the Metadata Module \n
 * 4. Read Primary Node ID for each Object \n
 * 5. Reply with Object and Primary List
 */
void Mds::downloadFileProcess(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path) {
	vector<uint64_t> objectList;
	vector<uint32_t> primaryList;
	uint64_t fileSize = 0;
	string checksum = "";
	FileType fileType = NORMAL;

	_nameSpaceModule->openFile(clientId, path);
	_metaDataModule->openFile(clientId, fileId);
	if (fileId != 0) {
		debug("Read object List %" PRIu32 "\n", fileId);
		objectList = _metaDataModule->readObjectList(fileId);

		vector<uint64_t>::iterator it;
		uint32_t primaryId;
		for (it = objectList.begin(); it < objectList.end(); ++it) {
			debug("Read primary list %" PRIu64 "\n", *it);
			try {
				primaryId = _metaDataModule->getPrimary(*it);
			} catch (...) {
				debug_yellow("%s\n", "No Primary Found");
				continue;
			}
			primaryList.push_back(primaryId);
		}
		objectList.resize(primaryList.size());

		fileSize = _metaDataModule->readFileSize(fileId);
		checksum = _metaDataModule->readChecksum(fileId);

		debug("FILESIZE = %" PRIu64 "\n", fileSize);
	} else
		fileType = NOTFOUND;

	_mdsCommunicator->replyDownloadInfo(requestId, connectionId, fileId, path,
			fileSize, fileType, checksum, objectList, primaryList);

	return;
}

/**
 * @brief	Handle Get Object ID Lsit
 */
void Mds::getObjectIdListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t numOfObjs) {
	vector<uint64_t> objectList = _metaDataModule->newObjectList(numOfObjs);
	vector<uint32_t> primaryList = _mdsCommunicator->getPrimaryList(
			_mdsCommunicator->getMonitorSockfd(), numOfObjs);
	_mdsCommunicator->replyObjectIdList(requestId, connectionId, objectList,
			primaryList);
	return;
}

/**
 * @brief	Handle Get File Info Request
 */
void Mds::getFileInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &path) {
	downloadFileProcessor(requestId, connectionId, clientId, path);

	return;
}

/**
 * @brief	Handle the Object Info Request from Osd
 * TODO: Currently Only Supplying Info Same as Download
 */
void Mds::getObjectInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId) {
	struct ObjectMetaData objectMetaData = _metaDataModule->readObjectInfo(
			objectId);
	_mdsCommunicator->replyObjectInfo(requestId, connectionId, objectId,
			objectMetaData._size, objectMetaData._nodeList,
			objectMetaData._codingScheme, objectMetaData._codingSetting);

	return;
}

/**
 * @brief	Handle List Folder Request from Client
 *
 * 1. List Folder with Name Space Module \n
 * 2. Reply with Folder Data
 */
void Mds::listFolderProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &path) {
	vector<FileMetaData> folderData;

	debug("List %s by %" PRIu32 "\n", path.c_str(), clientId);
	folderData = _nameSpaceModule->listFolder(clientId, path);
	_mdsCommunicator->replyFolderData(requestId, connectionId, path,
			folderData);

	return;
}

/**
 * @brief	Handle Primary Node Failure Report from Client
 *
 * 1. Select Acting Primary \n
 * 2. Report Node Failure to Monitor \n
 * 3. Reply with Acting Primary ID \n
 */
void Mds::primaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t osdId, uint64_t objectId, FailureReason reason) {

	// get the node list of an object and their status
	vector<uint32_t> nodeList = _metaDataModule->readNodeList(objectId);
	vector<bool> nodeStatus = _mdsCommunicator->getOsdStatusRequest(nodeList);

	// select new primary OSD and write to DB
	uint32_t actingPrimary = _metaDataModule->selectActingPrimary(objectId,
			nodeList, nodeStatus);

//	_mdsCommunicator->reportFailure(osdId, reason);

	_mdsCommunicator->replyPrimary(requestId, connectionId, objectId,
			actingPrimary);

	return;
}

/**
 * @brief	Handle Secondary Node Failure Report from Osd
 */
void Mds::secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t osdId, uint64_t objectId, FailureReason reason) {
	_mdsCommunicator->reportFailure(osdId, reason);

	return;
}

/**
 * @brief	Handle Osd Recovery Initialized by Monitor
 *
 * 1. Read Object List of the Failed Osd
 * 2. For each Object, Read Current Primary and Node List
 * 3. Reply with Object List, Primary List, and Node List of the Objects
 */
void Mds::recoveryTriggerProcessor(uint32_t requestId, uint32_t connectionId,
		vector<uint32_t> deadOsdList) {

	debug_yellow("%s\n", "HAHA");
	vector<struct ObjectLocation> objectLocationList;

	struct ObjectLocation objectLocation;

	for (uint32_t osdId : deadOsdList) {

		// get the list of objects owned by the failed osd as primary
		vector<uint64_t> primaryObjectList =
				_metaDataModule->readOsdPrimaryObjectList(osdId);

		for (auto objectId : primaryObjectList) {
			// get the node list of an object and their status
			vector<uint32_t> nodeList = _metaDataModule->readNodeList(objectId);
			vector<bool> nodeStatus = _mdsCommunicator->getOsdStatusRequest(
					nodeList);

			// select new primary OSD and write to DB
			_metaDataModule->selectActingPrimary(objectId, nodeList,
					nodeStatus);
		}

		// get the list of objects owned by failed osd
		vector<uint64_t> objectList = _metaDataModule->readOsdObjectList(osdId);

		for (auto objectId : objectList) {
			debug_cyan("Check objectid = %" PRIu64 "\n", objectId);
			objectLocation.objectId = objectId;
			objectLocation.osdList = _metaDataModule->readNodeList(objectId);
			objectLocation.primaryId = _metaDataModule->getPrimary(objectId);
			objectLocationList.push_back(objectLocation);
		}
	}

	_mdsCommunicator->replyRecoveryTrigger(requestId, connectionId,
			objectLocationList);

	return;
}

/**
 * @brief	Handle Object Node List Update from Osd
 */
void Mds::nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, const vector<uint32_t> &objectNodeList) {
	_metaDataModule->saveNodeList(objectId, objectNodeList);
	_metaDataModule->setPrimary(objectId, objectNodeList[0]);
	return;
}

/**
 * @brief	Handle Object List Save Request
 */
void Mds::saveObjectListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId,
		const vector<uint64_t> &objectList) {
	_metaDataModule->saveObjectList(fileId, objectList);
	_mdsCommunicator->replySaveObjectList(requestId, connectionId, fileId);
	return;
}

void Mds::repairObjectInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, vector<uint32_t> repairSegmentList,
		vector<uint32_t> repairSegmentOsdList) {

	struct ObjectMetaData objectMetaData = _metaDataModule->readObjectInfo(
			objectId);

	for (int i = 0; i < (int) repairSegmentList.size(); i++) {
		objectMetaData._nodeList[repairSegmentList[i]] = repairSegmentOsdList[i];
	}

	_metaDataModule->saveNodeList(objectId, objectMetaData._nodeList);
}

MdsCommunicator* Mds::getCommunicator() {
	return _mdsCommunicator;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

/**
 * @brief	Handle Set File Size Request
 */
void Mds::setFileSizeProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, uint64_t fileSize) {
	_metaDataModule->setFileSize(fileId, fileSize);

	return;
}

/**
 * @brief	Test Case
 */
void Mds::test() {
	/*
	 uint64_t objectId = 976172415;
	 struct ObjectMetaData objectMetaData = _metaDataModule->readObjectInfo(objectId);

	 debug("Object %" PRIu64 "- Coding %d:%s\n",objectId, (int)objectMetaData._codingScheme, objectMetaData._codingSetting.c_str());
	 for(const auto node : objectMetaData._nodeList) {
	 debug("%" PRIu32 "\n",node);
	 }
	 */
	/*
	 uint32_t fileId = 216;
	 vector <uint64_t> objectList = _metaDataModule->readObjectList(fileId);
	 debug("Object List [%" PRIu32 "]\n",fileId);
	 for(const auto object : objectList){
	 uint32_t primaryId = _metaDataModule->getPrimary(object);
	 printf("%" PRIu64 " [%" PRIu32 "]- ", object,primaryId);
	 }
	 printf("\n");
	 */
	/*
	 debug("%s\n", "Test\n");
	 for (int i = 0; i < 10; ++i) {
	 uint32_t temp = _metaDataModule->createFile(1, ".", 1024, RAID1_CODING);
	 vector<uint64_t> objectList;
	 objectList = _metaDataModule->newObjectList(10);
	 _metaDataModule->saveObjectList(temp, objectList);
	 for (int j = 0; j < 10; ++j) {
	 _metaDataModule->saveNodeList(objectList[j], { 1 });
	 _metaDataModule->setPrimary(objectList[j], 1);
	 }
	 }
	 */
}

int main(void) {
	configLayer = new ConfigLayer("mdsconfig.xml");

	mds = new Mds();

	MdsCommunicator* communicator = mds->getCommunicator();

	communicator->createServerSocket();
	communicator->setId(50000);
	communicator->setComponentType(MDS);

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	// 3. Send Thread
	thread sendThread(&Communicator::sendMessage, communicator);

	communicator->connectToMonitor();

	//mds->test();

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	delete mds;
	delete configLayer;
	return 0;
}
