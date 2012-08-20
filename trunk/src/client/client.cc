#include <signal.h>
#include <iostream>
#include <cstdio>
#include <thread>
#include <iomanip>
#include <chrono>
#include "client.hh"
#include "client_storagemodule.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../coding/raid1coding.hh"

using namespace std;

mutex pendingObjectChunkMutex;


// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT)
		exit(42);
}

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

Client::Client() {
	_clientCommunicator = new ClientCommunicator();
	_storageModule = new ClientStorageModule();

	_clientId = configLayer->getConfigInt("Clientid");

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
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

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
		_clientCommunicator->sendObject(_clientId, dstOsdSockfd, objectData,
				codingScheme, codingSetting);
		MemoryPool::getInstance().poolFree(objectData.buf);
	}

	debug("Upload %s Done [%" PRIu32 "]\n", path.c_str(), fileMetaData._id);

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1024.0;
	double fileSizeMb = fileSize / 1048576.0;
	double rate = fileSizeMb / duration;

	cout << fixed;
	cout << setprecision(2);
	cout << fileSizeMb << " MB transferred in " << duration << " secs, Rate = "
			<< rate << " MB/s" << endl;

	return fileMetaData._id;
}

void Client::downloadFileRequest(uint32_t fileId, string dstPath) {

	const uint64_t objectSize = _storageModule->getObjectSize();

	// start timer
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

	// 1. Get file infomation from MDS
	struct FileMetaData fileMetaData = _clientCommunicator->downloadFile(
			_clientId, fileId);
	debug("File ID %" PRIu32 ", Size = %" PRIu64 "\n",
			fileMetaData._id, fileMetaData._size);
	_storageModule->createFile(dstPath);

	// 2. Download file from OSD
	uint32_t i = 0;
	for (uint64_t objectId : fileMetaData._objectList) {
		uint32_t dstComponentId = fileMetaData._primaryList[i];
		uint32_t dstSockfd = _clientCommunicator->getSockfdFromId(
				dstComponentId);

		//TODO: get object in parallel
		struct ObjectData objectData = _clientCommunicator->getObject(_clientId,
				dstSockfd, objectId);

		// write to file
		const uint64_t offset = objectSize * i;
		struct ObjectCache objectCache = _storageModule->getObjectCache(objectId);
		_storageModule->writeFile(dstPath, objectCache.buf, offset, objectCache.length);
		debug ("Write Object ID: %" PRIu64 " Offset: %" PRIu64 " Length: %" PRIu64 " to %s\n", objectId, offset, objectCache.length, dstPath.c_str());

		i++;
	}

	_storageModule->closeFile(dstPath);

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1024.0;
	double fileSizeMb = fileMetaData._size / 1048576.0;
	double rate = fileSizeMb / duration;

	cout << fixed;
	cout << setprecision(2);
	cout << fileSizeMb << " MB transferred in " << duration << " secs, Rate = "
			<< rate << " MB/s" << endl;
}

void Client::putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount) {

	// initialize chunkCount value
	{
		lock_guard<mutex> lk(pendingObjectChunkMutex);
		_pendingObjectChunk[objectId] = chunkCount;
		debug ("Init Chunkcount = %" PRIu32 "\n", chunkCount);
	}

	// create object and cache
	updatePendingObjectChunkMap(objectId,chunkCount);
	_storageModule->createObject(objectId, length);
	_clientCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

}

void Client::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd, uint64_t objectId) {

	// TODO: check integrity of object received
	bool chunkRemaining = false;

	while (1) {

		{
			lock_guard<mutex> lk(pendingObjectChunkMutex);
			chunkRemaining = (bool) _pendingObjectChunk.count(objectId);
		}

		if (!chunkRemaining) {
			// if all chunks have arrived, send ack
			_storageModule->closeObject(objectId);
			_clientCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);
			break;
		} else {
			usleep(100000); // sleep 0.1s
		}

	}

}

uint32_t Client::ObjectDataProcessor(uint32_t requestId, uint32_t sockfd, uint64_t objectId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeObjectCache(objectId, buf, offset, length);
	{
		lock_guard<mutex> lk(pendingObjectChunkMutex);
		// update pendingObjectChunk value
		_pendingObjectChunk[objectId]--;
		debug ("Data Chunkcount = %" PRIu32 "\n", _pendingObjectChunk[objectId]);
	}

	/*
	// if all chunks have arrived
	if (chunkLeft == 0) {
		struct ObjectCache objectCache = _storageModule->getObjectCache(objectId);
		// write cache to disk
		byteWritten = _storageModule->writeObject(objectId, objectCache.buf, 0, objectCache.length);
		// close file and free cache
		_storageModule->closeObject(objectId);
		// Acknowledge MDS for Object Upload Completed
		// TODO
	}
	*/
	return byteWritten;
}

void Client::removePendingObjectFromMap(uint64_t objectId){
	lock_guard<mutex> lk(pendingObjectChunkMutex);
	_pendingObjectChunk.erase(objectId);
}

void Client::updatePendingObjectChunkMap(uint64_t objectId, uint32_t chunkCount){
	lock_guard<mutex> lk(pendingObjectChunkMutex);
	_pendingObjectChunk[objectId] = chunkCount;
}

void Client::setPendingChunkCount(uint64_t objectId, int32_t chunkCount){
	lock_guard<mutex> lk(pendingObjectChunkMutex);
	_pendingObjectChunk[objectId] = chunkCount;
}

uint32_t Client::getPendingChunkCount(uint64_t objectId){
	lock_guard<mutex> lk(pendingObjectChunkMutex);
	return _pendingObjectChunk[objectId];
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


int main(int argc, char *argv[]) {

	if (argc < 3 || argc > 4) {
		cout << "Upload: ./CLIENT upload [SRC]" << endl;
		cout << "Download: ./CLIENT download [FILEID] [DST]" << endl;
		exit (-1);
	}

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

	if (strcmp (argv[1], "upload") == 0) {
		client->uploadFileRequest(argv[2], codingScheme, codingSetting);
	} else {
		client->downloadFileRequest(atoi(argv[2]), argv[3]);
	}

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



