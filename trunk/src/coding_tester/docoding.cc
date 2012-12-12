/*
 * doCoding.cc
 */

#include <vector>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <iomanip>
#include "docoding.hh"
#include "storage.hh"
#include "../coding/coding.hh"
#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

extern Coding* coding;
extern string codingSetting;
extern string segmentFolder;
extern string objectFolder;

typedef chrono::high_resolution_clock Clock;
typedef chrono::milliseconds milliseconds;

void printResult(Clock::time_point tStart, Clock::time_point tRead,
		Clock::time_point tCode, Clock::time_point tWrite) {

	// calculate duration
	double durationRead = (chrono::duration_cast < milliseconds
			> (tRead - tStart)).count() / 1024.0;
	double durationCode = (chrono::duration_cast < milliseconds
			> (tCode - tRead)).count() / 1024.0;
	double durationWrite = (chrono::duration_cast < milliseconds
			> (tWrite - tCode)).count() / 1024.0;
	double durationTotal = (chrono::duration_cast < milliseconds
			> (tWrite - tStart)).count() / 1024.0;

	// output time result
	cout << fixed;
	cout << setprecision(2);
	cout << endl;
	cout << "Total Time Spent: " << durationTotal << " secs " << endl;
	cout << "Time Spent on Reading: " << durationRead << " secs " << endl;
	cout << "Time Spent on Coding: " << durationCode << " secs " << endl;
	cout << "Time Spent on Writing: " << durationWrite << " secs " << endl;
	cout << endl;

}

void doEncode(std::string srcObjectPath) {

	// start timer
	Clock::time_point tStart = Clock::now();

	ObjectData objectData;

	uint32_t filesize; // set by reference in readFile
	objectData.buf = readFile(srcObjectPath, filesize); // read object

	// fill in object information
	uint64_t objectId = (uint64_t) time(NULL); // time is used as objectID in testing program only
	objectData.info.objectSize = filesize;
	objectData.info.objectId = objectId;
	objectData.info.objectPath = srcObjectPath;

	cout << "Object ID: " << objectId << " Size: " << formatSize(filesize)
			<< endl;

	// take time for reading objects
	Clock::time_point tRead = Clock::now();

	// perform coding
	vector<SegmentData> segmentDataList = coding->encode(objectData,
			codingSetting);

	// take time for coding objects
	Clock::time_point tCode = Clock::now();

	// write object to storage
	const string objectPath = objectFolder + "/" + to_string(objectId);
	writeFile(objectPath, objectData.buf, objectData.info.objectSize);

	// write segment to storage
	cout << endl << "Writing Segments: " << endl;
	for (auto segment : segmentDataList) {
		const string segmentPath = segmentFolder + "/" + to_string(objectId)
				+ "." + to_string(segment.info.segmentId);

		writeFile(segmentPath, segment.buf, segment.info.segmentSize);

		cout << segment.info.segmentId << ": " << segmentPath << endl;

		// free segments
		MemoryPool::getInstance().poolFree(segment.buf);
	}

	// free object
	MemoryPool::getInstance().poolFree(objectData.buf);

	// take time for writing and clean up
	Clock::time_point tWrite = Clock::now();

	// print timing result
	printResult(tStart, tRead, tCode, tWrite);

	cout << endl << "Command for Decoding: " << endl
			<< "./CODING_TESTER decode " << objectId << " " << filesize
			<< " ./decoded_file" << endl;

}

void doDecode(uint64_t objectId, uint64_t objectSize, std::string dstObjectPath,
		uint32_t numSegments, vector<bool> secondaryOsdStatus) {

	// start timer
	Clock::time_point tStart = Clock::now();

	vector<SegmentData> segmentDataList(numSegments);

	// find required segments
	vector<uint32_t> requiredSegments = coding->getRequiredSegmentIds(
			codingSetting, secondaryOsdStatus);

	if (requiredSegments.size() == 0) {
		cerr << "Not enough segments to reconstruct file" << endl;
		return;
	}

	// read segments from files
	cout << "Reading Segments: " << endl;
	for (uint32_t i : requiredSegments) {
		const string segmentPath = segmentFolder + "/" + to_string(objectId)
				+ "." + to_string(i);

		SegmentData segmentData;
		uint32_t filesize; // set by reference in readFile
		segmentData.buf = readFile(segmentPath, filesize); // read segment

		// fill in segment information
		segmentData.info.objectId = objectId;
		segmentData.info.segmentId = i;
		segmentData.info.segmentSize = filesize;

		cout << i << ": " << segmentPath << " size = " << filesize << endl;

		segmentDataList[i] = segmentData;
	}

	// take time for reading objects
	Clock::time_point tRead = Clock::now();

	// perform decoding
	ObjectData objectData = coding->decode(segmentDataList, requiredSegments,
			objectSize, codingSetting);

	// take time for coding objects
	Clock::time_point tCode = Clock::now();

	// write object to dstObjectPath
	writeFile(dstObjectPath, objectData.buf, objectData.info.objectSize);

	// free object
	MemoryPool::getInstance().poolFree(objectData.buf);

	// free segments
	for (uint32_t i : requiredSegments) {
	//	cout << "Free-ing segment " << i << endl;
		MemoryPool::getInstance().poolFree(segmentDataList[i].buf);
	}

	// take time for writing and clean up
	Clock::time_point tWrite = Clock::now();

	// print timing result
	printResult(tStart, tRead, tCode, tWrite);

	cout << "Decoded object written to " << dstObjectPath << endl;

}

