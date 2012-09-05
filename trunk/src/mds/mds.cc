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
		uint32_t clientId, string dstPath, uint64_t fileSize,
		uint32_t numOfObjs, CodingScheme codingScheme, string codingSetting) {

	vector<uint64_t> objectList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;

	_nameSpaceModule->createFile(clientId, dstPath);
	fileId = _metaDataModule->createFile(clientId, dstPath, fileSize, codingScheme, codingSetting);

	objectList = _metaDataModule->newObjectList(numOfObjs);
	_metaDataModule->saveObjectList(fileId, objectList);

	
//	primaryList = _mdsCommunicator->askPrimaryList(numOfObjs);
	primaryList = _mdsCommunicator->getPrimaryList(_mdsCommunicator->getMonitorSockfd(), numOfObjs);
	for (uint32_t i = 0; i < primaryList.size(); i++) {
		debug("Get primary list index %" PRIu32 " = %" PRIu32 "\n", i, primaryList[i]);
	}

	_mdsCommunicator->replyObjectandPrimaryList(requestId, connectionId, fileId,
			objectList, primaryList);

	return fileId;
}

/**
 * @brief	Handle Upload Object Acknowledgement from Primary
 *
 * 1. Save the Node List of the object \n
 * 2. Set the Primary for the object
 */
void Mds::uploadObjectAckProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, CodingScheme codingScheme, string codingSetting, vector<uint32_t> objectNodeList) {
	struct ObjectMetaData objectMetaData;
	objectMetaData._id = objectId;
	objectMetaData._nodeList = objectNodeList;
	objectMetaData._primary = objectNodeList[0];
	objectMetaData._codingScheme = codingScheme;
	objectMetaData._codingSetting = codingSetting;
	_metaDataModule->saveObjectInfo(objectId, objectMetaData);
	//_metaDataModule->saveNodeList(objectId, objectNodeList);
	//_metaDataModule->setPrimary(objectId, objectNodeList[0]);

	return;
}

/**
 * @brief	Handle Download File Request from Client (Request with Path)
 */
void Mds::downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, string dstPath) {
	uint32_t fileId = _metaDataModule->lookupFileId(dstPath);
	debug("Path = %s [%" PRIu32 "]\n",dstPath.c_str(), fileId);
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
		uint32_t clientId, uint32_t fileId, string path) {
	vector<uint64_t> objectList;
	vector<uint32_t> primaryList;
	uint64_t fileSize = 0;
	string checksum = "";

	_nameSpaceModule->openFile(clientId, path);
	_metaDataModule->openFile(clientId, fileId);
	if(fileId != 0) {
		debug("Read object List %" PRIu32 "\n", fileId);
		objectList = _metaDataModule->readObjectList(fileId);

		vector<uint64_t>::iterator it;
		uint32_t primaryId;
		for (it = objectList.begin(); it < objectList.end(); ++it) {
			debug("Read primary list %" PRIu64 "\n", *it);
			primaryId = _metaDataModule->getPrimary(*it);
			primaryList.push_back(primaryId);
		}

		fileSize = _metaDataModule->readFileSize(fileId);
		checksum = _metaDataModule->readChecksum(fileId);

		debug ("FILESIZE = %" PRIu64 "\n", fileSize);\
	}
	_mdsCommunicator->replyDownloadInfo(requestId, connectionId, fileId, path, fileSize, checksum, objectList, primaryList);

	return;
}

/**
 * @brief	Handle Get Object ID Lsit
 */
void Mds::getObjectIdListProcessor(uint32_t requestId, uint32_t connectionId, uint32_t numOfObjs)
{
	vector<uint64_t> objectList = _metaDataModule->newObjectList(numOfObjs);
	_mdsCommunicator->replyObjectIdList(requestId, connectionId, objectList);
	return ;
}

/**
 * @brief	Handle Get File Info Request
 */
void Mds::getFileInfoProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, string path)
{
	downloadFileProcessor(requestId, connectionId, clientId, path);	

	return ;
}

/**
 * @brief	Handle the Object Info Request from Osd
 * TODO: Currently Only Supplying Info Same as Download
 */
void Mds::getObjectInfoProcessor(uint32_t requestId, uint32_t connectionId, uint64_t objectId)
{
	struct ObjectMetaData objectMetaData = _metaDataModule->readObjectInfo(objectId);
	_mdsCommunicator->replyObjectInfo(requestId, connectionId, objectId, objectMetaData._nodeList, objectMetaData._codingScheme, objectMetaData._codingSetting);

	return;
}

/**
 * @brief	Handle List Folder Request from Client
 *
 * 1. List Folder with Name Space Module \n
 * 2. Reply with Folder Data
 */
void Mds::listFolderProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, string path) {
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
	uint32_t actingPrimary = _metaDataModule->selectActingPrimary(objectId,
			osdId);
	_mdsCommunicator->reportFailure(osdId, reason);
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
 * 1. Read Object List of the Failed Osd \n
 * 2. For each Object, Read Current Primary and Node List \n
 * 3. Reply with Object List, Primary List, and Node List of the Objects
 */
void Mds::recoveryProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t osdId) {
	vector<uint64_t> objectList;
	vector<uint32_t> primaryList;
	vector<vector<uint32_t> > objectNodeList;

	_metaDataModule->readOsdObjectList(osdId);

	vector<uint64_t>::iterator it;
	uint32_t primaryId;

	for (it = objectList.begin(); it < objectList.end(); ++it) {
		primaryId = _metaDataModule->getPrimary(*it);
		primaryList.push_back(primaryId);

		vector<uint32_t> nodeList;
		nodeList = _metaDataModule->readNodeList(*it);
		objectNodeList.push_back(nodeList);
	}

	_mdsCommunicator->replyRecoveryInfo(requestId, connectionId, osdId,
			objectList, primaryList, objectNodeList);

	return;
}

/**
 * @brief	Handle Object Node List Update from Osd
 */
void Mds::nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, vector<uint32_t> objectNodeList) {
	_metaDataModule->saveNodeList(objectId, objectNodeList);
	_metaDataModule->setPrimary(objectId, objectNodeList[0]);
	return;
}

/**
 * @brief	Handle Object List Save Request
 */
void Mds::saveObjectListProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList)
{
	_metaDataModule->saveObjectList(fileId, objectList);
	return ;
}

MdsCommunicator* Mds::getCommunicator() {
	return _mdsCommunicator;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	mds->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	// wait for message
	communicator->waitForMessage();
}

/**
 * @brief	Handle Set File Size Request
 */
void Mds::setFileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, uint64_t fileSize)
{
	_metaDataModule->setFileSize(fileId, fileSize);	

	return ;
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
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	communicator->connectToMonitor();

	mds->test();

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	delete mds;
	delete configLayer;
	return 0;
}
