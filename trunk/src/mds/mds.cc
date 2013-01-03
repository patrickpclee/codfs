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

/// MDS Segment
Mds* mds;

/// Config Layer
ConfigLayer* configLayer;

/**
 * Initialise MDS Communicator and MetaData Modules
 */
Mds::Mds() {
	_hotnessModule = new HotnessModule();
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
	_mdsCommunicator = new MdsCommunicator();
}

Mds::~Mds() {
	delete _mdsCommunicator;
	delete _metaDataModule;
	delete _nameSpaceModule;
	delete _hotnessModule;
}

/**
 * @brief	Handle File Upload Request From Client
 *
 * 1. Create File in the Name Space (Directory Tree) \n
 * 2. Create File Meta Data (Generate File ID) \n
 * 3. Generate Segment IDs \n
 * 4. Ask Monitor for Primary list \n
 * 5. Reply with Segment and Primary List
 */
uint32_t Mds::uploadFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, const string &dstPath, uint64_t fileSize,
		uint32_t numOfObjs) {

	vector<uint64_t> segmentList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;

	_nameSpaceModule->createFile(clientId, dstPath);
	fileId = _metaDataModule->createFile(clientId, dstPath, fileSize);

	segmentList = _metaDataModule->newSegmentList(numOfObjs);
	_metaDataModule->saveSegmentList(fileId, segmentList);

	//	primaryList = _mdsCommunicator->askPrimaryList(numOfObjs);
	primaryList = _mdsCommunicator->getPrimaryList(
			_mdsCommunicator->getMonitorSockfd(), numOfObjs);
	for (uint32_t i = 0; i < primaryList.size(); i++) {
		debug("Get primary list index %" PRIu32 " = %" PRIu32 "\n",
				i, primaryList[i]);
	}

	_mdsCommunicator->replySegmentandPrimaryList(requestId, connectionId,
			fileId, segmentList, primaryList);

	return fileId;
}

void Mds::reportDeleteCacheProcessor(uint32_t requestId, uint32_t connectionId,
		list<uint64_t> segmentIdList, uint32_t osdId) {

	string deletedCacheString;
	for (auto segmentId : segmentIdList) {
		deletedCacheString += to_string(segmentId) + " ";
	}
	debug_yellow("Deleted Cache for OSD %" PRIu32 " = %s\n", osdId, deletedCacheString.c_str());
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
 * @brief	Handle File Rename Request From Client
 */
void Mds::renameFileProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path,
		const string &newPath) {

	string tmpPath = path;
	if (fileId != 0)
		tmpPath = _metaDataModule->lookupFilePath(fileId);
	else
		fileId = _metaDataModule->lookupFileId(tmpPath);

	_nameSpaceModule->renameFile(clientId, tmpPath, newPath);
	_metaDataModule->renameFile(clientId, fileId, newPath);
	_mdsCommunicator->replyRenameFile(requestId, connectionId, fileId, tmpPath);
}

/**
 * @brief	Handle Upload Segment Acknowledgement from Primary
 *
 * 1. Save the Node List of the segment \n
 * 2. Set the Primary for the segment
 */
/*
   void Mds::FileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t fileId){
   uint64_t fileSize = _metaDataModule->readFileSize(fileId);
   _mdsCommunicator->replyFileSize(requestId, connectionId, fileId, fileSize);
   }
 */

void Mds::uploadSegmentAckProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, uint32_t segmentSize, CodingScheme codingScheme,
		const string &codingSetting, const vector<uint32_t> &segmentNodeList,
		const string &checksum) {
	struct SegmentMetaData segmentMetaData;
	segmentMetaData._id = segmentId;
	segmentMetaData._nodeList = segmentNodeList;
	segmentMetaData._primary = segmentNodeList[0];
	segmentMetaData._codingScheme = codingScheme;
	segmentMetaData._codingSetting = codingSetting;
	segmentMetaData._checksum = checksum;
	segmentMetaData._size = segmentSize;
	_metaDataModule->saveSegmentInfo(segmentId, segmentMetaData);
	//_metaDataModule->saveNodeList(segmentId, segmentNodeList);
	//_metaDataModule->setPrimary(segmentId, segmentNodeList[0]);

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
 * 3. Read Segment List from the Metadata Module \n
 * 4. Read Primary Node ID for each Segment \n
 * 5. Reply with Segment and Primary List
 */
void Mds::downloadFileProcess(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId, const string &path) {
	vector<uint64_t> segmentList;
	vector<uint32_t> primaryList;
	uint64_t fileSize = 0;
	string checksum = "";
	FileType fileType = NORMAL;

	_nameSpaceModule->openFile(clientId, path);
	_metaDataModule->openFile(clientId, fileId);
	if (fileId != 0) {
		debug("Read segment List %" PRIu32 "\n", fileId);
		segmentList = _metaDataModule->readSegmentList(fileId);

		vector<uint64_t>::iterator it;
		uint32_t primaryId;
		for (it = segmentList.begin(); it < segmentList.end(); ++it) {
			debug("Read primary list %" PRIu64 "\n", *it);
			try {
				primaryId = _metaDataModule->getPrimary(*it);
			} catch (...) {
				debug_yellow("%s\n", "No Primary Found");
				continue;
			}
			primaryList.push_back(primaryId);
		}
		segmentList.resize(primaryList.size());

		fileSize = _metaDataModule->readFileSize(fileId);
		checksum = _metaDataModule->readChecksum(fileId);

		debug("FILESIZE = %" PRIu64 "\n", fileSize);
	} else
		fileType = NOTFOUND;

	_mdsCommunicator->replyDownloadInfo(requestId, connectionId, fileId, path,
			fileSize, fileType, checksum, segmentList, primaryList);

	return;
}

/**
 * @brief	Handle Get Segment ID Lsit
 */
void Mds::getSegmentIdListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t numOfObjs) {
	vector<uint64_t> segmentList = _metaDataModule->newSegmentList(numOfObjs);
	vector<uint32_t> primaryList = _mdsCommunicator->getPrimaryList(
			_mdsCommunicator->getMonitorSockfd(), numOfObjs);
	_mdsCommunicator->replySegmentIdList(requestId, connectionId, segmentList,
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
 * @brief	Handle the Segment Info Request from Osd
 * TODO: Currently Only Supplying Info Same as Download
 */
void Mds::getSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId) {
	struct SegmentMetaData segmentMetaData = _metaDataModule->readSegmentInfo(
			segmentId);
	_mdsCommunicator->replySegmentInfo(requestId, connectionId, segmentId,
			segmentMetaData._size, segmentMetaData._nodeList,
			segmentMetaData._codingScheme, segmentMetaData._codingSetting);

	// Hotness update and see whether new cache should be requested
	struct HotnessRequset req = _hotnessModule->updateSegmentHotness(segmentId,
			DEFAULT_HOTNESS_ALG, 0);

	// Check whether new cache should be issued
	if (req.numOfNewCache > 0) {
		// Issue the cache request
		vector<uint32_t> newAdded = _mdsCommunicator->requestCache(segmentId, req);

		// Update the cache list
		_hotnessModule->updateSegmentCache(segmentId, newAdded);
	}

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
		uint32_t osdId, uint64_t segmentId, FailureReason reason) {

	// get the node list of an segment and their status
	vector<uint32_t> nodeList = _metaDataModule->readNodeList(segmentId);
	vector<bool> nodeStatus = _mdsCommunicator->getOsdStatusRequest(nodeList);

	// select new primary OSD and write to DB
	uint32_t actingPrimary = _metaDataModule->selectActingPrimary(segmentId,
			nodeList, nodeStatus);

	//	_mdsCommunicator->reportFailure(osdId, reason);

	_mdsCommunicator->replyPrimary(requestId, connectionId, segmentId,
			actingPrimary);

	return;
}

/**
 * @brief	Handle Secondary Node Failure Report from Osd
 */
void Mds::secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t osdId, uint64_t segmentId, FailureReason reason) {
	_mdsCommunicator->reportFailure(osdId, reason);

	return;
}

/**
 * @brief	Handle Osd Recovery Initialized by Monitor
 *
 * 1. Read Segment List of the Failed Osd
 * 2. For each Segment, Read Current Primary and Node List
 * 3. Reply with Segment List, Primary List, and Node List of the Segments
 */
void Mds::recoveryTriggerProcessor(uint32_t requestId, uint32_t connectionId,
		vector<uint32_t> deadOsdList) {

	debug_yellow("%s\n", "Recovery Triggered");
	vector<struct SegmentLocation> segmentLocationList;

	struct SegmentLocation segmentLocation;

	for (uint32_t osdId : deadOsdList) {

		// get the list of segments owned by the failed osd as primary
		vector<uint64_t> primarySegmentList =
			_metaDataModule->readOsdPrimarySegmentList(osdId);

		for (auto segmentId : primarySegmentList) {
			// get the node list of an segment and their status
			vector<uint32_t> nodeList = _metaDataModule->readNodeList(
					segmentId);
			vector<bool> nodeStatus = _mdsCommunicator->getOsdStatusRequest(
					nodeList);

			// select new primary OSD and write to DB
			_metaDataModule->selectActingPrimary(segmentId, nodeList,
					nodeStatus);
		}

		// get the list of segments owned by failed osd
		vector<uint64_t> segmentList = _metaDataModule->readOsdSegmentList(
				osdId);

		for (auto segmentId : segmentList) {
			debug_cyan("Check segmentid = %" PRIu64 "\n", segmentId);
			segmentLocation.segmentId = segmentId;
			segmentLocation.osdList = _metaDataModule->readNodeList(segmentId);
			segmentLocation.primaryId = _metaDataModule->getPrimary(segmentId);
			segmentLocationList.push_back(segmentLocation);
		}
	}

	_mdsCommunicator->replyRecoveryTrigger(requestId, connectionId,
			segmentLocationList);

	return;
}

/**
 * @brief	Handle Segment Node List Update from Osd
 */
void Mds::nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, const vector<uint32_t> &segmentNodeList) {
	_metaDataModule->saveNodeList(segmentId, segmentNodeList);
	_metaDataModule->setPrimary(segmentId, segmentNodeList[0]);
	return;
}

/**
 * @brief	Handle Segment List Save Request
 */
void Mds::saveSegmentListProcessor(uint32_t requestId, uint32_t connectionId,
		uint32_t clientId, uint32_t fileId,
		const vector<uint64_t> &segmentList) {
	_metaDataModule->saveSegmentList(fileId, segmentList);
	_mdsCommunicator->replySaveSegmentList(requestId, connectionId, fileId);
	return;
}

void Mds::repairSegmentInfoProcessor(uint32_t requestId, uint32_t connectionId,
		uint64_t segmentId, vector<uint32_t> repairBlockList,
		vector<uint32_t> repairBlockOsdList) {

	struct SegmentMetaData segmentMetaData = _metaDataModule->readSegmentInfo(
			segmentId);

	for (int i = 0; i < (int) repairBlockList.size(); i++) {
		segmentMetaData._nodeList[repairBlockList[i]] = repairBlockOsdList[i];
	}

	_metaDataModule->saveNodeList(segmentId, segmentMetaData._nodeList);
}

MdsCommunicator* Mds::getCommunicator() {
	return _mdsCommunicator;
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
	   uint64_t segmentId = 976172415;
	   struct SegmentMetaData segmentMetaData = _metaDataModule->readSegmentInfo(segmentId);

	   debug("Segment %" PRIu64 "- Coding %d:%s\n",segmentId, (int)segmentMetaData._codingScheme, segmentMetaData._codingSetting.c_str());
	   for(const auto node : segmentMetaData._nodeList) {
	   debug("%" PRIu32 "\n",node);
	   }
	 */
	/*
	   uint32_t fileId = 216;
	   vector <uint64_t> segmentList = _metaDataModule->readSegmentList(fileId);
	   debug("Segment List [%" PRIu32 "]\n",fileId);
	   for(const auto segment : segmentList){
	   uint32_t primaryId = _metaDataModule->getPrimary(segment);
	   printf("%" PRIu64 " [%" PRIu32 "]- ", segment,primaryId);
	   }
	   printf("\n");
	 */
	/*
	   debug("%s\n", "Test\n");
	   for (int i = 0; i < 10; ++i) {
	   uint32_t temp = _metaDataModule->createFile(1, ".", 1024, RAID1_CODING);
	   vector<uint64_t> segmentList;
	   segmentList = _metaDataModule->newSegmentList(10);
	   _metaDataModule->saveSegmentList(temp, segmentList);
	   for (int j = 0; j < 10; ++j) {
	   _metaDataModule->saveNodeList(segmentList[j], { 1 });
	   _metaDataModule->setPrimary(segmentList[j], 1);
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

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	// 3. Send Thread
#ifdef USE_MULTIPLE_QUEUE
#else
	thread sendThread(&Communicator::sendMessage, communicator);
#endif

	communicator->connectToMonitor();

	garbageCollectionThread.join();
	receiveThread.join();
#ifdef USE_MULTIPLE_QUEUE
#else
	sendThread.join();
#endif

	delete mds;
	delete configLayer;
	return 0;
}
