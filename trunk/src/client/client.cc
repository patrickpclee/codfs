#include <cstdio>
#include <thread>

#include "client.hh"
#include "client_storagemodule.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

Client::Client() {
	_clientCommunicator = new ClientCommunicator();
	_storageModule = new ClientStorageModule();

	// TODO: HARDCODE CLIENT ID
	_clientId = 0;

}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* Client::getCommunicator() {
	return _clientCommunicator;
}

/**
 * 1. Divide the file into fixed size objects
 * 2. For each object, contact MDS to obtain objectId and dstOsdID
 * 3. Call uploadObjectRequest()
 */

uint32_t Client::uploadFileRequest(string filepath) {

	const uint32_t objectCount = _storageModule->getObjectCount(filepath);

	debug("Object Count of %s is %" PRIu32 "\n", filepath.c_str(), objectCount);

	for (uint32_t i = 0; i < objectCount; ++i) {
		struct ObjectData objectData = _storageModule->readObjectFromFile(
				filepath, i);

		// TODO: HARDCODE FOR NOW!
		uint32_t dstOsdId = _clientCommunicator->getOsdSockfd();
		objectData.info.objectId = i;

		_clientCommunicator->putObject(_clientId, dstOsdId, objectData);
	}

	return 0;
}

/*
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
	//	communicator->connectToMds();

	// connect to OSD
	communicator->connectToOsd();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	////////////////////// TEST FUNCTIONS ////////////////////////////

	// TEST PUT OBJECT
	client->uploadFileRequest("./testfile");

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

