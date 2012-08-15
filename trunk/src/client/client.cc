#include <signal.h>

#include <iostream>
#include <cstdio>
#include <thread>
#include <iomanip>
#include <sys/time.h>
#include "client.hh"
#include "client_storagemodule.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../coding/raid1coding.hh"

#include <sys/stat.h>

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT)
		exit(42);
}

// record time
double getTime () {
	struct timeval tp;
	double sec, usec;
	gettimeofday(&tp, NULL);
	sec = static_cast<double>(tp.tv_sec);
	usec = static_cast<double>(tp.tv_usec) / 1E6;
	return sec + usec;
}

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

Client::Client() {
	_clientCommunicator = new ClientCommunicator();
	_storageModule = new ClientStorageModule();

	// TODO: HARDCODE CLIENT ID
	_clientId = 1;

}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* Client::getCommunicator() {
	return _clientCommunicator;
}

uint32_t Client::uploadFileRequest(string path, CodingScheme codingScheme,
		string codingSetting) {

	// start timer
	double start = getTime();

	const uint32_t objectCount = _storageModule->getObjectCount(path);
	const uint64_t fileSize = _storageModule->getFilesize(path);

	debug("Object Count of %s: %" PRIu32 "\n", path.c_str(), objectCount);

	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(
			_clientId, path, fileSize, objectCount, codingScheme,
			codingSetting);

	debug("File ID %" PRIu32 "\n", fileMetaData._id);

	debug("%s\n", "====================");
	for (uint32_t i = 0; i < fileMetaData._primaryList.size(); ++i) {
		debug("%" PRIu64 "[%" PRIu32 "]\n",
				fileMetaData._objectList[i], fileMetaData._primaryList[i]);
	}

	for (uint32_t i = 0; i < objectCount; ++i) {
		struct ObjectData objectData = _storageModule->readObjectFromFile(path,
				i);
		uint32_t primary = fileMetaData._primaryList[i];
		debug("Send to Primary [%" PRIu32 "]\n", primary);
		// TODO: HARDCODE FOR NOW!
		//uint32_t dstOsdSockfd = _clientCommunicator->getOsdSockfd();
		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
		objectData.info.objectId = fileMetaData._objectList[i];
		_clientCommunicator->putObject(_clientId, dstOsdSockfd, objectData,
				codingScheme, codingSetting);
	}

	debug("Upload %s Done [%" PRIu32 "]\n", path.c_str(), fileMetaData._id);

	// Time and Rate calculation (in seconds)
	double end = getTime();
	double duration = end - start;
	double fileSizeMb = fileSize / 1048576.0;
	double rate = fileSizeMb / duration;

	cout << fixed;
	cout << setprecision(2);
	cout << fileSizeMb  << " MB transferred in " << duration << " secs, Rate = "
			<< rate << " MB/s" << endl;

	return fileMetaData._id;
}

/**
 * 1. Divide the file into fixed size objects
 * 2. For each object, contact MDS to obtain objectId and dstOsdID
 * 3. Call uploadObjectRequest()
 */

/*
 uint32_t Client::sendFileRequest(string filepath, CodingScheme codingScheme, string codingSetting) {

 // start timer
 struct timeval tp;
 double sec, usec, start, end;
 gettimeofday(&tp, NULL);
 sec = static_cast<double>(tp.tv_sec);
 usec = static_cast<double>(tp.tv_usec) / 1E6;
 start = sec + usec;

 const uint32_t objectCount = _storageModule->getObjectCount(filepath);

 // TODO: obtain a list of OSD for upload from MDS

 debug("Object Count of %s is %" PRIu32 "\n", filepath.c_str(), objectCount);

 for (uint32_t i = 0; i < objectCount; ++i) {
 struct ObjectData objectData = _storageModule->readObjectFromFile(
 filepath, i);

 // TODO: HARDCODE FOR NOW!
 uint32_t dstOsdSockfd = _clientCommunicator->getOsdSockfd();
 objectData.info.objectId = i;

 _clientCommunicator->putObject(_clientId, dstOsdSockfd, objectData,
 codingScheme, codingSetting);
 }

 // Time stamp after the computations
 gettimeofday(&tp, NULL);
 sec = static_cast<double>(tp.tv_sec);
 usec = static_cast<double>(tp.tv_usec) / 1E6;
 end = sec + usec;

 // Time and Rate calculation (in seconds)
 double duration = end - start;
 uint64_t filesize = _storageModule->getFilesize(filepath) / 1024.0 / 1024.0;
 double rate = filesize / duration;

 cout << fixed;
 cout << setprecision(2);
 cout << filesize << " MB transferred in "
 << duration << " secs, Rate = " << rate << " MB/s" << endl;

 return 0;
 }

 uint32_t Client::downloadFileRequest(string dstPath) {
 return 0;
 }
 */

void Client::downloadFileRequest(uint32_t fileId, string dstPath) {
	// to be implemented
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	client->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	// wait for message
	communicator->waitForMessage();

}

int main(void) {

	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client();
	ClientCommunicator* communicator = client->getCommunicator();

	// start server
	communicator->createServerSocket();

	/*
	 const int segmentNumber = configLayer->getConfigInt("Coding>SegmentNumber");
	 debug("Segment Number = %d\n", segmentNumber);
	 */

	// connect to MDS
	//communicator->connectToMds();
	//communicator->connectToOsd();
	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	communicator->setId(51000);
	communicator->setComponentType(CLIENT);
	communicator->connectAllComponents();
	////////////////////// TEST FUNCTIONS ////////////////////////////

	// TEST PUT OBJECT

	CodingScheme codingScheme = RAID1_CODING;
	string codingSetting = Raid1Coding::generateSetting(3);

	//client->sendFileRequest("./testfile", codingScheme, codingSetting);
	client->uploadFileRequest("./testfile", codingScheme, codingSetting);

	/*

	 // TEST LIST FOLDER
	 vector<FileMetaData> folderData;
	 folderData = communicator->listFolderData(1, ".");

	 // TODO: when to delete listFolderDataRequest and listFolderDataReply?

	 vector<FileMetaData>::iterator it;
	 for (it = folderData.begin(); it < folderData.end(); ++it) {
	 debug("name: %s size: %d\n", ((*it)._path).c_str(), (int)(*it)._size);
	 }
	 */

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	return 0;
}

