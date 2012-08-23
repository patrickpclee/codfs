#include <iostream>
#include <signal.h>
#include "client.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../coding/raid1coding.hh"

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT)
		exit(42);
}

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

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

	if (argc < 4 || argc > 5) {
		cout << "Upload: ./CLIENT [CONFIG] upload [SRC]" << endl;
		cout << "Download: ./CLIENT [CONFIG] download [FILEID] [DST]" << endl;
		exit(-1);
	}

	// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer(argv[1]);
	client = new Client();
	ClientCommunicator* communicator = client->getCommunicator();

	// start server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	communicator->setId(client->getClientId());
	communicator->setComponentType(CLIENT);
	communicator->connectAllComponents();

	////////////////////// TEST FUNCTIONS ////////////////////////////

	// TEST PUT OBJECT

	const uint32_t replicationFactor = 1;

	if (strncmp(argv[2], "upload", 6) == 0) {
		CodingScheme codingScheme = RAID1_CODING;
		string codingSetting = Raid1Coding::generateSetting(replicationFactor);
		client->uploadFileRequest(argv[3], codingScheme, codingSetting);
	} else {
		client->downloadFileRequest(atoi(argv[3]), argv[4]);
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
