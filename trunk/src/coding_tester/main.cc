#include <iostream>
#include <algorithm>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <boost/lexical_cast.hpp>
#include "../config/config.hh"
#include "../coding/coding.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/evenoddcoding.hh"
#include "../coding/rdpcoding.hh"
#include "../coding/rscoding.hh"
#include "../coding/cauchycoding.hh"
#include "../coding/embrcoding.hh"
#include "../common/convertor.hh"
#include "../common/debug.hh"
#include "docoding.hh"

using namespace std;

// Global Variables
ConfigLayer* configLayer;
string blockFolder;
string segmentFolder;
string repairFolder;
Coding* coding;
string codingSetting;
uint32_t numFailedOsd;
uint64_t segmentSize;

void printUsage() {
	cout << "Encode: ./coding_tester encode [SRC_SEGMENT_PATH]" << endl;
	cout
		<< "Decode: ./coding_tester decode [SEGMENT_ID] [SEGMENT_SIZE] [DST_SEGMENT_PATH]"
		<< endl;
	cout << "Repair: ./coding_tester repair [SEGMENT_ID] [SEGMENT_SIZE]"
		<< endl;
}

void printOsdStatus(vector<bool> secondaryOsdStatus) {
	cout << endl << "===== OSD HEALTH SIMULATION =====" << endl;
	for (int i = 0; i < (int) secondaryOsdStatus.size(); i++) {
		cout << "OSD " << i << " : " << secondaryOsdStatus[i] << endl;
	}
	cout << "=================================" << endl;
}

void createFolder (string folder) {
	// create folder if not exist
	struct stat st;
	if(stat(folder.c_str(),&st) != 0) {
		cout << folder << " does not exist, make directory automatically" << endl;
		if (mkdir (folder.c_str(), S_IRWXU | S_IRGRP | S_IROTH) < 0) {
			perror ("mkdir");
			exit (-1);
		}
	}
}

uint32_t readConfig(const char* configFile) {

	uint32_t numBlocks;

	cout << endl << "=================================" << endl;

	// read storage location
	segmentFolder = string(configLayer->getConfigString("SegmentFolder"));
	cout << "Segment Location: " << segmentFolder << endl;
	createFolder (segmentFolder);

	blockFolder = string(configLayer->getConfigString("BlockFolder"));
	cout << "Block Location: " << blockFolder << endl;
	createFolder (blockFolder);

	repairFolder = string(configLayer->getConfigString("RepairFolder"));
	cout << "Repair Location: " << repairFolder << endl;
	createFolder (repairFolder);

	// read coding configuration
	string selectedCoding = string(
			configLayer->getConfigString("SelectedCodingScheme"));

	if (selectedCoding == "RAID0") {

		int raid0_n = configLayer->getConfigInt("CodingSetting>RAID0>n");
		coding = new Raid0Coding();
		codingSetting = Raid0Coding::generateSetting(raid0_n);
		numBlocks = raid0_n;
		cout << "Coding: RAID 0, n = " << raid0_n << endl;

	} else if (selectedCoding == "RAID1") {

		int raid1_n = configLayer->getConfigInt("CodingSetting>RAID1>n");
		coding = new Raid1Coding();
		codingSetting = Raid1Coding::generateSetting(raid1_n);
		numBlocks = raid1_n;
		cout << "Coding: RAID 1, n = " << raid1_n << endl;

	} else if (selectedCoding == "RAID5") {

		int raid5_n = configLayer->getConfigInt("CodingSetting>RAID5>n");
		coding = new Raid5Coding();
		codingSetting = Raid5Coding::generateSetting(raid5_n);
		numBlocks = raid5_n;
		cout << "Coding: RAID 5, n = " << raid5_n << endl;

	} else if (selectedCoding == "EVENODD") {

		int n = configLayer->getConfigInt("CodingSetting>EVENODD>n");
		coding = new EvenOddCoding();
		codingSetting = EvenOddCoding::generateSetting(n);
		numBlocks = coding->getBlockCountFromSetting(codingSetting);
		cout << "Coding: Even Odd, n = " << n << endl;

	} else if (selectedCoding == "RDP") {

		int n = configLayer->getConfigInt("CodingSetting>RDP>n");
		coding = new RDPCoding();
		codingSetting = RDPCoding::generateSetting(n);
		numBlocks = coding->getBlockCountFromSetting(codingSetting);
		cout << "Coding: RDP, n = " << n << endl;

	} else if (selectedCoding == "RS") {

		int k = configLayer->getConfigInt("CodingSetting>RS>k");
		int m = configLayer->getConfigInt("CodingSetting>RS>m");
		int w = configLayer->getConfigInt("CodingSetting>RS>w");
		coding = new RSCoding();
		codingSetting = RSCoding::generateSetting((uint32_t) k, (uint32_t) m,
				(uint32_t) w);
		numBlocks = k + m;
		cout << "Coding: Reed Solomon, k = " << k << " m = " << m << " w = "
			<< w << endl;

	} else if (selectedCoding == "CAUCHY") {

		int k = configLayer->getConfigInt("CodingSetting>CAUCHY>k");
		int m = configLayer->getConfigInt("CodingSetting>CAUCHY>m");
		int w = configLayer->getConfigInt("CodingSetting>CAUCHY>w");
		coding = new CauchyCoding();
		codingSetting = CauchyCoding::generateSetting((uint32_t) k, (uint32_t) m,
				(uint32_t) w);
		numBlocks = k + m;
		cout << "Coding: Cauchy RS, k = " << k << " m = " << m << " w = " << w << endl;


	} else if (selectedCoding == "EMBR") {

		int n = configLayer->getConfigInt("CodingSetting>EMBR>n");
		int k = configLayer->getConfigInt("CodingSetting>EMBR>k");
		int w = configLayer->getConfigInt("CodingSetting>EMBR>w");
		coding = new EMBRCoding();
		codingSetting = EMBRCoding::generateSetting((uint32_t) n, (uint32_t) k,
				(uint32_t) w);
		numBlocks = n;
		cout << "Coding: E-MBR, n = " << n << " k = " << k << " w = " << w << endl;
	} else {

		cerr << "Wrong Coding Scheme Specified!" << endl;
		exit(-1);

	}

	cout << "=================================" << endl << endl;

	return numBlocks;
}

int main(int argc, char* argv[]) {

	// check arguments
	if (argc < 3 || argc > 5) {
		printUsage();
		exit(0);
	}

	// read config file
	configLayer = new ConfigLayer("coding_tester_config.xml");
	uint32_t numBlocks = readConfig("coding_tester_config.xml");

	if (string(argv[1]) == "encode") {

		const string srcSegmentPath = argv[2];
		cout << "Encoding Segment: " << srcSegmentPath << endl;

		doEncode(srcSegmentPath);

	} else if (string(argv[1]) == "decode") {

		const uint64_t segmentId = boost::lexical_cast<uint64_t>(argv[2]);
		const uint64_t segmentSize = boost::lexical_cast<uint64_t>(argv[3]);
		string dstSegmentPath = argv[4];
		cout << "Decoding Segment ID: " << segmentId << " size = "
			<< segmentSize << " to " << dstSegmentPath << endl;

		// OSD status array, true = ONLINE, false = OFFLINE
		vector<bool> secondaryOsdStatus(numBlocks, true);

		numFailedOsd = configLayer->getConfigInt("NumFailedOsd");
		/*
		// get the number of failed OSD from config
		if (numFailedOsd > numBlocks) {
		cerr << "Number of Failed OSD > Number of Blocks" << endl;
		exit(0);
		}

		// Simulate OSD Failure
		vector<uint32_t> shuffleArray(numBlocks);
		for (uint32_t i = 0; i < numBlocks; i++) {
		shuffleArray[i] = i;
		}

		// randomly set the specified number of OSD to FAIL
		srand(time(NULL));
		random_shuffle(shuffleArray.begin(), shuffleArray.end());
		for (uint32_t i = 0; i < numFailedOsd; i++) {
		secondaryOsdStatus[shuffleArray[i]] = false;
		}

		cout << "Randomly failed " << numFailedOsd << " OSDs" << endl;
		 */
		string ori_dstSegmentPath = dstSegmentPath;
		if(numFailedOsd == 1) {
			for(uint32_t i = 0; i < numBlocks; ++i) {
				secondaryOsdStatus[i] = false;
				dstSegmentPath = ori_dstSegmentPath + "_" + to_string(i);
				printOsdStatus(secondaryOsdStatus);

				doDecode(segmentId, segmentSize, dstSegmentPath, numBlocks,
						secondaryOsdStatus);

				secondaryOsdStatus[i] = true;
			}
		} else if(numFailedOsd == 2) {
			for(uint32_t i = 0; i < numBlocks; ++i) {
				secondaryOsdStatus[i] = false;
				for(uint32_t j = i + 1; j < numBlocks; ++j) {
					secondaryOsdStatus[j] = false;
					dstSegmentPath = ori_dstSegmentPath + "_" + to_string(i) + "-" + to_string(j);
					printOsdStatus(secondaryOsdStatus);

					doDecode(segmentId, segmentSize, dstSegmentPath, numBlocks,
							secondaryOsdStatus);

					secondaryOsdStatus[j] = true;
				}
				secondaryOsdStatus[i] = true;
			}
		} else {
			printOsdStatus(secondaryOsdStatus);

			doDecode(segmentId, segmentSize, dstSegmentPath, numBlocks,
					secondaryOsdStatus);
		}

	} else if (string(argv[1]) == "repair") {
		const uint64_t segmentId = boost::lexical_cast<uint64_t>(argv[2]);
		const uint64_t segmentSize = boost::lexical_cast<uint64_t>(argv[3]);

		cout << "Decoding Segment ID: " << segmentId << " size = "
			<< segmentSize << endl;

		// OSD status array, true = ONLINE, false = OFFLINE
		vector<bool> secondaryOsdStatus(numBlocks, true);

		// get the number of failed OSD from config
		numFailedOsd = configLayer->getConfigInt("NumFailedOsd");
		if (numFailedOsd > numBlocks) {
			cerr << "Number of Failed OSD > Number of Blocks" << endl;
			exit(0);
		}

		// Simulate OSD Failure
		// randomly set the specified number of OSD to FAIL
		/*
		   srand(time(NULL));
		   uint32_t j = 0;
		   while (j < numFailedOsd) {
		   uint32_t idx = rand() % numBlocks;
		   if (secondaryOsdStatus[idx] == true) {
		   secondaryOsdStatus[idx] = false;
		   j++;
		   }
		   }
		 */
		if(numFailedOsd == 1) {
			for(uint32_t i = 0; i < numBlocks; ++i) {
				secondaryOsdStatus[i] = false;
				cout << "Failed " << numFailedOsd << " OSDs" << endl;

				printOsdStatus(secondaryOsdStatus);

				vector<string> dstBlockPaths;
				cout << "Repair Location: " << repairFolder << endl;

				for (uint32_t k = 0; k < numBlocks; k++) {
					if (secondaryOsdStatus[k] == false) {
						dstBlockPaths.push_back (repairFolder + "/" + to_string(segmentId) + "." + to_string(k));
					}
				}

				doRepair(segmentId, segmentSize, numBlocks, secondaryOsdStatus, dstBlockPaths);
				secondaryOsdStatus[i] = true;
			}
		} else if(numFailedOsd == 2) {
			for(uint32_t i = 0; i < numBlocks; ++i) {
				secondaryOsdStatus[i] = false;
				for(uint32_t j = i + 1; j < numBlocks; ++j) {
					secondaryOsdStatus[j] = false;

					cout << "Failed " << numFailedOsd << " OSDs" << endl;

					printOsdStatus(secondaryOsdStatus);

					vector<string> dstBlockPaths;
					cout << "Repair Location: " << repairFolder << endl;

					for (uint32_t k = 0; k < numBlocks; k++) {
						if (secondaryOsdStatus[k] == false) {
							dstBlockPaths.push_back (repairFolder + "/" + to_string(segmentId) + "." + to_string(k));
						}
					}

					doRepair(segmentId, segmentSize, numBlocks, secondaryOsdStatus, dstBlockPaths);

					secondaryOsdStatus[j] = true;
				}
				secondaryOsdStatus[i] = true;
			}
		}
	}

	return 0;
}

