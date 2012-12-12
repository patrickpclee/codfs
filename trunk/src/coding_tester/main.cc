#include <iostream>
#include <algorithm>
#include <ctime>
#include "config/config.hh"
#include "lib/lexical_cast.hpp"
#include "../coding/coding.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"
#include "../common/convertor.hh"
#include "../common/debug.hh"
#include "docoding.hh"

using namespace std;

// Global Variables
ConfigLayer* configLayer;
string blockFolder;
string segmentFolder;
Coding* coding;
string codingSetting;
uint32_t numFailedOsd;
uint64_t segmentSize;

void printUsage() {
	cout << "Encode: ./coding_tester encode [SRC_SEGMENT_PATH]" << endl;
	cout
			<< "Decode: ./coding_tester decode [SEGMENT_ID] [SEGMENT_SIZE] [DST_SEGMENT_PATH]"
			<< endl;
}

void printOsdStatus(vector<bool> secondaryOsdStatus) {
	cout << endl << "===== OSD HEALTH SIMULATION =====" << endl;
	for (int i = 0; i < (int) secondaryOsdStatus.size(); i++) {
		cout << "OSD " << i << " : " << secondaryOsdStatus[i] << endl;
	}
	cout << "=================================" << endl;
}

uint32_t readConfig(const char* configFile) {

	uint32_t numBlocks;

	cout << endl << "=================================" << endl;

	// read storage location
	segmentFolder = string(configLayer->getConfigString("SegmentFolder"));
	cout << "Segment Location: " << segmentFolder << endl;

	blockFolder = string(configLayer->getConfigString("BlockFolder"));
	cout << "Block Location: " << blockFolder << endl;

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

	} else if (selectedCoding == "RS") {

		int k = configLayer->getConfigInt("CodingSetting>RS>k");
		int m = configLayer->getConfigInt("CodingSetting>RS>m");
		int w = configLayer->getConfigInt("CodingSetting>RS>w");
		coding = new RSCoding();
		codingSetting = RSCoding::generateSetting((uint32_t)k,(uint32_t)m,(uint32_t)w);
		numBlocks = k+m;
		cout << "Coding: Reed Solomon, k = " << k <<" m = " << m << " w = " << w  << endl;
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
		const string dstSegmentPath = argv[4];
		cout << "Decoding Segment ID: " << segmentId << " size = " << segmentSize
				<< " to " << dstSegmentPath << endl;

		// OSD status array, true = ONLINE, false = OFFLINE
		vector<bool> secondaryOsdStatus(numBlocks, true);

		// get the number of failed OSD from config
		numFailedOsd = configLayer->getConfigInt("NumFailedOsd");
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

		printOsdStatus(secondaryOsdStatus);

		doDecode(segmentId, segmentSize, dstSegmentPath, numBlocks, secondaryOsdStatus);
	}

	return 0;
}

