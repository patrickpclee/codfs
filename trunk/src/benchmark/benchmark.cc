#include <iostream>
#include <signal.h>
#include <chrono>
#include <openssl/md5.h>
#include <boost/thread/thread.hpp>

#include "../client/client.hh"
#include "../client/client_storagemodule.hh"

#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../common/convertor.hh"
#include "../common/define.hh"

#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"

#include "../../lib/threadpool/threadpool.hpp"

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		exit(42);
	}
}

/// Client Object
Client* client;

ClientCommunicator* _clientCommunicator;

ClientStorageModule* _storageModule;

/// Config Layer
ConfigLayer* configLayer;

uint32_t clientId;
string fileName;
uint32_t fileId;
CodingScheme codingScheme;
string codingSetting;
uint64_t fileSize;
uint32_t objectSize;
uint32_t numberOfObject;
char* databuf;
unsigned char* checksum;

bool download;

#ifdef PARALLEL_TRANSFER
void startBenchUploadThread(uint32_t clientId, uint32_t sockfd,
		struct ObjectData objectData, CodingScheme codingScheme,
		string codingSetting, string checksum) {
	_clientCommunicator->sendObject(clientId, sockfd, objectData,
			codingScheme, codingSetting, checksum);
}

void startBenchDownloadThread(uint32_t clientId, uint32_t sockfd, uint64_t objectId) {
	client->getObject(clientId, sockfd, objectId);
	_storageModule->closeObject(objectId);
}

boost::threadpool::pool _tp;
#endif

void parseOption(int argc, char* argv[]){
	clientId = atoi(argv[1]);
	
	if(strcmp(argv[2], "download") == 0){
		download = true;
		fileId = atoi(argv[3]); 
		debug("Downloading File %"PRIu32"\n", fileId);
	} else {

		fileName = time(NULL);

		fileSize = stringToByte(argv[3]);
		objectSize = stringToByte(argv[4]);
		numberOfObject = fileSize / objectSize;

		debug("Testing %s with File Size %" PRIu64 " Object Size %"PRIu32"\n",argv[2],fileSize,objectSize);

		codingScheme = RAID1_CODING;
		codingSetting = Raid1Coding::generateSetting(1);
	}


#ifdef PARALLEL_TRANSFER
	uint32_t _numClientThreads = configLayer->getConfigInt(
			"Communication>NumClientThreads");
	_tp.size_controller().resize(_numClientThreads);
#endif
}

void prepareData(){
	databuf = (char*)calloc(objectSize,1);
	checksum = (unsigned char*)calloc(MD5_DIGEST_LENGTH,1);
	int* intptr = (int*)databuf;
	for(uint32_t i = 0; i < objectSize / 4; ++i) {
		intptr[i] = rand();
	}
	MD5((unsigned char*) databuf, objectSize, checksum);
}

void testDownload() {
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();
	struct FileMetaData fileMetaData = _clientCommunicator->downloadFile(clientId, fileId);
	debug("File ID %" PRIu32 ", Size = %" PRIu64 "\n", fileMetaData._id, fileMetaData._size);

	for(uint32_t i = 0; i < fileMetaData._objectList.size(); ++i) {
		uint32_t primary = fileMetaData._primaryList[i];
		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
		uint64_t objectId = fileMetaData._objectList[i];
#ifdef PARALLEL_TRANSFER
		_tp.schedule(boost::bind(startBenchDownloadThread, clientId, dstOsdSockfd, objectId));
#else
		client->getObject(clientId, sockfd, objectId);
		_storageModule->closeObject(objectId);
#endif
	}

#ifdef PARALLEL_TRANSFER
	_tp.wait();
#endif

	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(10000);

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileMetaData._size) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileMetaData._size / duration)
			<< "/s" << endl;
}

void testUpload() {
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(clientId, fileName, fileSize, numberOfObject, codingScheme, codingSetting);

	debug("File ID %" PRIu32 "\n", fileMetaData._id);

	for(uint32_t i = 0; i < numberOfObject; ++i){
		struct ObjectData objectData;
		objectData.buf = databuf;
		objectData.info.objectId = fileMetaData._objectList[i];
		objectData.info.objectSize = objectSize;
		uint32_t primary = fileMetaData._primaryList[i];
		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
#ifdef PARALLEL_TRANSFER
		_tp.schedule(boost::bind(startBenchUploadThread, clientId, dstOsdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum)));
#else
		_clientCommunicator->sendObject(clientId, dstOsdSockfd, objectData, codingScheme, codingSetting, md5ToHex(checksum));
#endif
	}
#ifdef PARALLEL_TRANSFER
	_tp.wait();
#endif

	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(10000);

	cout << "Upload Done [" << fileMetaData._id << "]" << endl;

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileSize) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileSize / duration) << "/s"
			<< endl;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	client->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* _clientCommunicator) {
	// wait for message
	_clientCommunicator->waitForMessage();
}


int main(int argc, char *argv[]) {

	if (argc < 4) {
		cout << "Upload: ./BENCHMARK [ID] upload [FILESIZE] [OBJECTSIZE]" << endl;
		cout << "Download: ./BENCHMARK [ID] download [FILEID]" << endl;
		exit(-1);
	}
	// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client(clientId);
	_clientCommunicator = client->getCommunicator();
	_storageModule = client->getStorageModule();

	parseOption(argc, argv);
	prepareData();

	// start server
	_clientCommunicator->createServerSocket();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, _clientCommunicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	_clientCommunicator->setId(client->getClientId());
	_clientCommunicator->setComponentType(CLIENT);
	
	//_clientCommunicator->connectAllComponents();
	_clientCommunicator->connectToMds();
	_clientCommunicator->connectToMonitor();
	_clientCommunicator->getOsdListAndConnect();

	if(download)
		testDownload();
	else
		testUpload();

	/*

	// RAID 0
	const uint32_t raid0_n = 3;
	CodingScheme codingScheme = RAID0_CODING;
	string codingSetting = Raid0Coding::generateSetting(raid0_n);

	// RAID 1
	const uint32_t raid1_n = 3;
	CodingScheme codingScheme = RAID1_CODING;
	string codingSetting = Raid1Coding::generateSetting(raid1_n);

	// RAID 5
	const uint32_t raid5_n = 3;
	CodingScheme codingScheme = RAID5_CODING;
	string codingSetting = Raid5Coding::generateSetting(raid5_n);

	*/

	/*
	// RS
	const uint32_t rs_k = 6, rs_m = 2, rs_w = 8;
	CodingScheme codingScheme = RS_CODING;
	string codingSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);

	if (strncmp(argv[2], "upload", 6) == 0) {
		client->uploadFileRequest(argv[3], codingScheme, codingSetting);
	} else {
		client->downloadFileRequest(atoi(argv[3]), argv[4]);
	}
	*/

	cout << "Now Sleep 5 Seconds then Exit" << endl;
	sleep(5);
	exit(0);
	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	return 0;
}
