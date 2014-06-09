#include <iostream>
#include <signal.h>
#include <chrono>
#include <ctime>
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
#include "../coding/embrcoding.hh"
#include "../coding/rdpcoding.hh"

#include "../../lib/threadpool/threadpool.hpp"

#define DATA_BUF_SIZE 256435456ULL

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT) {
		debug_yellow("%s\n", "SIGINT received\n");
		exit(42);
	}
}

/// Client Segment
Client* client;

ClientCommunicator* _clientCommunicator;

ClientStorageModule* _storageModule;

/// Config Layer
ConfigLayer* configLayer;

uint32_t clientId;
string fileName;
uint32_t fileId;
uint32_t startPercent, endPercent, iteration;
uint32_t replicationPercent;
CodingScheme codingScheme;
string codingSetting;
uint64_t fileSize;
uint32_t segmentSize;
uint32_t numberOfSegment;
char* databuf;

bool download;
bool hotness = false;

void startBenchUploadThread(uint32_t clientId, uint32_t sockfd,
		struct SegmentData segmentData, CodingScheme codingScheme,
		string codingSetting) {
	_clientCommunicator->sendSegment(clientId, sockfd, segmentData,
			codingScheme, codingSetting);
}

void startBenchDownloadThread(uint32_t clientId, uint32_t sockfd,
		uint64_t segmentId) {
	SegmentData segmentCache = client->getSegment(clientId,
			sockfd, segmentId);
	_storageModule->closeSegment(segmentId);
}

boost::threadpool::pool _tp;

void parseOption(int argc, char* argv[]) {

	if (strcmp (argv[2], "hotness") == 0) {
		if (argc != 4) {
			cout << "Hotness: ./BENCHMARK [ID] hotness [FILEID]" << endl;
			exit(-1);
		}
		startPercent = 0;
		endPercent = 100;
		iteration = 1;
		download = true;
		hotness = true;
		fileId = atoi(argv[3]);
	} else if (strcmp(argv[2], "download") == 0) {
		if (argc != 4 && argc != 7) {
			cout
					<< "Download: ./BENCHMARK [ID] download [FILEID] [START PERCENT] [END PERCENT] [ITERATION]"
					<< endl;
			exit(-1);
		} else if (argc == 4) {
			// backward compatible
			startPercent = 0;
			endPercent = 100;
			iteration = 1;
		} else {
			startPercent = atoi(argv[4]);
			endPercent = atoi(argv[5]);
			iteration = atoi(argv[6]);
		}
		download = true;
		fileId = atoi(argv[3]);
		debug(
				"Downloading File %" PRIu32 " start = %d end = %d iteration = %d\n",
				fileId, (int) startPercent, (int) endPercent, (int)iteration);
	} else {

		if (argc < 6) {
			cout
					<< "Upload: ./BENCHMARK [ID] upload [FILESIZE] [SEGMENTSIZE] [CODING_TEST] [REPLICATION PERCENT]"
					<< endl;
			exit(-1);
		}

		fileName = to_string(time(NULL));

		fileSize = stringToByte(argv[3]);
		segmentSize = stringToByte(argv[4]);
		numberOfSegment = fileSize / segmentSize;

		if (argc == 7) {
			replicationPercent = atoi (argv[6]);
		} else {
			replicationPercent = 0;
		}

		debug(
				"Testing %s with File Size %" PRIu64 " Segment Size %" PRIu32 "\n",
				argv[2], fileSize, segmentSize);

		int benchmarkTest = atoi(argv[5]);

		switch (benchmarkTest) {
		case 0:
			codingScheme = RAID0_CODING;
			codingSetting = Raid0Coding::generateSetting(5);
			break;
		case 1:
			codingScheme = RAID1_CODING;
			codingSetting = Raid1Coding::generateSetting(1);
			break;
		case 2:
			codingScheme = RAID1_CODING;
			codingSetting = Raid1Coding::generateSetting(2);
			break;
		case 3:
			codingScheme = RAID1_CODING;
			codingSetting = Raid1Coding::generateSetting(3);
			break;
		case 4:
			codingScheme = RAID5_CODING;
			codingSetting = Raid5Coding::generateSetting(6);
			break;
		case 5:
			codingScheme = RS_CODING;
			codingSetting = RSCoding::generateSetting(4, 2, 8);
			break;
		case 6:
			codingScheme = EMBR_CODING;
			codingSetting = EMBRCoding::generateSetting(4, 2, 8);
			break;
		case 7:
			codingScheme = RDP_CODING;
			codingSetting = RDPCoding::generateSetting(6);
			break;
		case 8:
			codingScheme = EVENODD_CODING;
			codingSetting = EvenOddCoding::generateSetting(7);
			break;
		default:
			debug("Invalid Test = %d\n", benchmarkTest);
			break;
		}
		debug("Coding Setting: %s\n", codingSetting.c_str());
	}

	uint32_t _numClientThreads = configLayer->getConfigInt(
			"Communication>NumClientThreads");
	_tp.size_controller().resize(_numClientThreads);
}

void prepareData() {
	databuf = (char*) calloc(DATA_BUF_SIZE, 1);
	int* intptr = (int*) databuf;
	for (uint32_t i = 0; i < DATA_BUF_SIZE / 4; ++i) {
		intptr[i] = rand();
	}
}

void testDownload() {

	cout << "[BENCHMARK START] " << getTime() << endl;

	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();
	struct FileMetaData fileMetaData = _clientCommunicator->downloadFile(
			clientId, fileId);
	debug("File ID %" PRIu32 ", Size = %" PRIu64 "\n",
			fileMetaData._id, fileMetaData._size);

	double startPortion = startPercent / (double) 100;
	double endPortion = endPercent / (double) 100;
	debug ("StartPortion = %f, endPortion = %f\n", startPortion, endPortion);

	uint32_t segmentCount = fileMetaData._segmentList.size();

	if (hotness) {
		int i = 0;
		vector <int> requestList;
		while (scanf ("%d", &i) == 1) {
			requestList.push_back(i);
		}
		random_shuffle(requestList.begin(), requestList.end());

		for (int i : requestList) {
			uint32_t primary = fileMetaData._primaryList[i];
			uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
			uint64_t segmentId = fileMetaData._segmentList[i];
			debug("segmentId = %" PRIu64 " primary = %" PRIu32 "\n",
					segmentId, primary);

			_tp.schedule(
					boost::bind(startBenchDownloadThread, clientId, dstOsdSockfd,
							segmentId));
		}
	} else {

		vector <uint64_t> segmentList (fileMetaData._segmentList.size());
		vector <uint32_t> primaryList (fileMetaData._primaryList.size());
		vector<int> shuffleList (segmentList.size());
		for (int i = 0; i < (int)shuffleList.size(); i++) {
			shuffleList[i] = i;
		}

#ifdef RANDOM_SHUFFLE_SEGMENT_ORDER
		std::random_shuffle ( shuffleList.begin(), shuffleList.end() );
#endif

		for (int i = 0; i < (int)shuffleList.size(); i++) {
			segmentList[i] = fileMetaData._segmentList[shuffleList[i]];
			primaryList[i] = fileMetaData._primaryList[shuffleList[i]];
		}

		for (uint32_t i = 0; i < segmentCount; ++i) {

#ifndef RANDOM_SHUFFLE_SEGMENT_ORDER
			if (i / (double) segmentCount < startPortion
					|| i / (double) segmentCount > endPortion) {
				continue; // skip this segment
			}
#endif

			uint32_t primary = primaryList[i];
			uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
			uint64_t segmentId = segmentList[i];

			debug("segmentId = %" PRIu64 " primary = %" PRIu32 "\n",
					segmentId, primary);

			_tp.schedule(
					boost::bind(startBenchDownloadThread, clientId, dstOsdSockfd,
							segmentId));
		}
	}

	_tp.wait();

	Clock::time_point t1 = Clock::now();

	cout << "[BENCHMARK END] " << getTime() << endl;

	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(USLEEP_DURATION);

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileMetaData._size) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileMetaData._size / duration)
			<< "/s" << endl;
}

void testUpload() {

	int dataBufIdx[numberOfSegment];
	for (uint32_t i = 0; i < numberOfSegment; ++i) {
		dataBufIdx[i] = rand() % (DATA_BUF_SIZE - segmentSize);
	}

	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;

	cout << "[BENCHMARK START] " << getTime() << endl;

	Clock::time_point t0 = Clock::now();

	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(clientId,
			fileName, fileSize, numberOfSegment);

	debug("File ID %" PRIu32 "\n", fileMetaData._id);

	map<uint32_t, uint32_t> OSDLoadCount;
	for (uint32_t i = 0; i < numberOfSegment; ++i) {
		struct SegmentData segmentData;
		segmentData.buf = databuf + dataBufIdx[i];
		segmentData.info.segmentId = fileMetaData._segmentList[i];
		segmentData.info.segLength = segmentSize;
		uint32_t primary = fileMetaData._primaryList[i];

		if (OSDLoadCount.count(primary) == 0)
			OSDLoadCount[primary] = 1;
		else
			OSDLoadCount[primary] += 1;

		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);

		CodingScheme currentCodingScheme = codingScheme;
		string currentCodingSetting = codingSetting;

		double replicationPortion = replicationPercent / (double) 100;

		if ((double)i / numberOfSegment < replicationPortion) {
			currentCodingScheme = RAID1_CODING;
			currentCodingSetting = Raid1Coding::generateSetting(1);
		}

		_tp.schedule(
				boost::bind(startBenchUploadThread, clientId, dstOsdSockfd,
						segmentData, currentCodingScheme, currentCodingSetting));
	}
	_tp.wait();

	Clock::time_point t1 = Clock::now();

	cout << "[BENCHMARK END] " << getTime() << endl;

	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(USLEEP_DURATION);

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileSize) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileSize / duration) << "/s"
			<< endl;

	map<uint32_t, uint32_t>::iterator it;

	for (it = OSDLoadCount.begin(); it != OSDLoadCount.end(); ++it) {
		cout << (*it).first << ":" << (*it).second << endl;
	}
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startReceiveThread(Communicator* _clientCommunicator) {
	// wait for message
	_clientCommunicator->waitForMessage();
}

int main(int argc, char *argv[]) {

	// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	clientId = atoi(argv[1]);
	client = new Client(clientId);
	_clientCommunicator = client->getCommunicator();
	_storageModule = client->getStorageModule();

	srand(time(NULL) + clientId);

	parseOption(argc, argv);
	prepareData();

	// start server
	_clientCommunicator->createServerSocket();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, _clientCommunicator);

	_clientCommunicator->setId(client->getClientId());
	_clientCommunicator->setComponentType(CLIENT);

	//_clientCommunicator->connectAllComponents();
	_clientCommunicator->connectToMds();
	_clientCommunicator->connectToMonitor();
	_clientCommunicator->getOsdListAndConnect();

	if (download)
		if (hotness) {
			testDownload();
		} else {
			for (uint32_t i = 0; i < iteration; i++) {
				cout << "START ITERATION " << i << endl;
				testDownload();
				cout << "END ITERATION " << i << endl;
			}
		}
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

	return 0;
}
