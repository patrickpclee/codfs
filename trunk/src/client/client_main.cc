#include <iostream>
#include <signal.h>
#include "client.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"
#include "../../lib/logger.hh"

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
		cout << "Upload: ./CLIENT [CLIENT ID] upload [SRC]" << endl;
		cout << "Download: ./CLIENT [CLIENT ID] download [FILEID] [DST]" << endl;
		exit(-1);
	}

	// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client(atoi(argv[1]));
	ClientCommunicator* communicator = client->getCommunicator();

	// setup log
	FILELog::ReportingLevel() = logDEBUG3;
	std::string logFileName = "client_" + to_string(client->getClientId()) + ".log";
	FILE* log_fd = fopen(logFileName.c_str(), "a");
	Output2FILE::Stream() = log_fd;

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
	
	//communicator->connectAllComponents();
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->getOsdListAndConnect();


	////////////////////// TEST FUNCTIONS ////////////////////////////

	// TEST LOGGING

	// TEST PUT OBJECT

	/*

	// RAID 0
	const uint32_t raid0_n = 3;
	CodingScheme codingScheme = RAID0_CODING;
	string codingSetting = Raid0Coding::generateSetting(raid0_n);
*/

	// RAID 1
	const uint32_t raid1_n = 1;
	CodingScheme codingScheme = RAID1_CODING;
	string codingSetting = Raid1Coding::generateSetting(raid1_n);

/*
	// RAID 5
	const uint32_t raid5_n = 3;
	CodingScheme codingScheme = RAID5_CODING;
	string codingSetting = Raid5Coding::generateSetting(raid5_n);


	// RS
	const uint32_t rs_k = 6, rs_m = 2, rs_w = 8;
	CodingScheme codingScheme = RS_CODING;
	string codingSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);
	*/

	if (strncmp(argv[2], "upload", 6) == 0) {
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
