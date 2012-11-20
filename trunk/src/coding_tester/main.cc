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
string segmentFolder;
string objectFolder;
Coding* coding;
string codingSetting;
uint32_t numFailedOsd;
uint64_t objectSize;

void printUsage() {
	cout << "Encode: ./coding_tester encode [SRC_OBJECT_PATH]" << endl;
	cout
			<< "Decode: ./coding_tester decode [OBJECT_ID] [OBJECT_SIZE] [DST_OBJECT_PATH]"
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

	uint32_t numSegments;

	cout << endl << "=================================" << endl;

	// read storage location
	objectFolder = string(configLayer->getConfigString("ObjectFolder"));
	cout << "Object Location: " << objectFolder << endl;

	segmentFolder = string(configLayer->getConfigString("SegmentFolder"));
	cout << "Segment Location: " << segmentFolder << endl;

	// read coding configuration
	string selectedCoding = string(
			configLayer->getConfigString("SelectedCodingScheme"));

	if (selectedCoding == "RAID0") {

		int raid0_n = configLayer->getConfigInt("CodingSetting>RAID0>n");
		coding = new Raid0Coding();
		codingSetting = Raid0Coding::generateSetting(raid0_n);
		numSegments = raid0_n;
		cout << "Coding: RAID 0, n = " << raid0_n << endl;

	} else if (selectedCoding == "RAID1") {

		int raid1_n = configLayer->getConfigInt("CodingSetting>RAID1>n");
		coding = new Raid1Coding();
		codingSetting = Raid1Coding::generateSetting(raid1_n);
		numSegments = raid1_n;
		cout << "Coding: RAID 1, n = " << raid1_n << endl;


	} else if (selectedCoding == "RAID5") {

		int raid5_n = configLayer->getConfigInt("CodingSetting>RAID5>n");
		coding = new Raid5Coding();
		codingSetting = Raid5Coding::generateSetting(raid5_n);
		numSegments = raid5_n;
		cout << "Coding: RAID 5, n = " << raid5_n << endl;

	} else if (selectedCoding == "RS") {

		int k = configLayer->getConfigInt("CodingSetting>RS>k");
		int m = configLayer->getConfigInt("CodingSetting>RS>m");
		int w = configLayer->getConfigInt("CodingSetting>RS>w");
		coding = new RSCoding();
		codingSetting = RSCoding::generateSetting((uint32_t)k,(uint32_t)m,(uint32_t)w);
		numSegments = k+m;
		cout << "Coding: Reed Solomon, k = " << k <<" m = " << m << " w = " << w  << endl;
	} else {

		cerr << "Wrong Coding Scheme Specified!" << endl;
		exit(-1);

	}

	cout << "=================================" << endl << endl;

	return numSegments;
}

int main(int argc, char* argv[]) {

	// check arguments
	if (argc < 3 || argc > 5) {
		printUsage();
		exit(0);
	}

	// read config file
	configLayer = new ConfigLayer("config.xml");
	uint32_t numSegments = readConfig("config.xml");

	if (string(argv[1]) == "encode") {

		const string srcObjectPath = argv[2];
		cout << "Encoding Object: " << srcObjectPath << endl;

		doEncode(srcObjectPath);

	} else if (string(argv[1]) == "decode") {

		const uint64_t objectId = boost::lexical_cast<uint64_t>(argv[2]);
		const uint64_t objectSize = boost::lexical_cast<uint64_t>(argv[3]);
		const string dstObjectPath = argv[4];
		cout << "Decoding Object ID: " << objectId << " size = " << objectSize
				<< " to " << dstObjectPath << endl;

		// OSD status array, true = ONLINE, false = OFFLINE
		vector<bool> secondaryOsdStatus(numSegments, true);

		// get the number of failed OSD from config
		numFailedOsd = configLayer->getConfigInt("NumFailedOsd");
		if (numFailedOsd > numSegments) {
			cerr << "Number of Failed OSD > Number of Segments" << endl;
			exit(0);
		}

		// Simulate OSD Failure
		vector<uint32_t> shuffleArray(numSegments);
		for (uint32_t i = 0; i < numSegments; i++) {
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

		doDecode(objectId, objectSize, dstObjectPath, numSegments, secondaryOsdStatus);
	}

	return 0;
}

