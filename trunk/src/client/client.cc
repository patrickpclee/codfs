#include <cstdio>
#include <thread>

#include "client.hh"
#include "client_storagemodule.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

// HARDCODE FOR TESTING

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

	debug("Object Count of %s = %d\n", filepath.c_str(), objectCount);

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

uint32_t Client::downloadFileRequest(string dstPath) {
	return 0;
}

uint32_t Client:: downloadFileRequest(uint32_t fileId) {
	return 0;
}

void sendThread() {
	debug("%s", "Send Thread Start\n");
	client->getCommunicator()->sendMessage();
	debug("%s", "Send Thread End\n");
}

void sleepThread(Client* client, ClientCommunicator* communicator) {

	usleep(2000000);

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
}

int main(void) {

	configLayer = new ConfigLayer("clientconfig.xml");

	client = new Client();

	ClientCommunicator* communicator = client->getCommunicator();

	const uint16_t serverPort = configLayer->getConfigInt(
			"Communication>ServerPort");

	debug("Start server on port %d\n", serverPort);

	const int segmentNumber = configLayer->getConfigInt("Coding>SegmentNumber");
	debug("Segment Number = %d\n", segmentNumber);

	communicator->createServerSocket(serverPort);

	// connect to MDS
//	communicator->connectToMds();

// connect to OSD
	communicator->connectToOsd();

	thread t(sendThread);
	t.detach();
	thread t1(sleepThread, client, communicator);
	t1.detach();

	// wait for message
	communicator->waitForMessage();

	printf("CLIENT\n");
	return 0;
}

