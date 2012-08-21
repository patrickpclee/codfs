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

	if (argc < 3 || argc > 4) {
		cout << "Upload: ./CLIENT upload [SRC]" << endl;
		cout << "Download: ./CLIENT download [FILEID] [DST]" << endl;
		exit(-1);
	}

	// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
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

	if (strcmp(argv[1], "upload") == 0) {
		CodingScheme codingScheme = RAID1_CODING;
		string codingSetting = Raid1Coding::generateSetting(3);
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
