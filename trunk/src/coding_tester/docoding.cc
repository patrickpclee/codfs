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
#include "../common/segmentdata.hh"
#include "../common/blockdata.hh"
#include "../common/memorypool.hh"

using namespace std;

extern Coding* coding;
extern string codingSetting;
extern string blockFolder;
extern string segmentFolder;

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

void doEncode(std::string srcSegmentPath) {

	// start timer
	Clock::time_point tStart = Clock::now();

	SegmentData segmentData;

	uint32_t filesize; // set by reference in readFile
	segmentData.buf = readFile(srcSegmentPath, filesize); // read segment

	// fill in segment information
	uint64_t segmentId = (uint64_t) time(NULL); // time is used as segmentID in testing program only
	segmentData.info.segmentSize = filesize;
	segmentData.info.segmentId = segmentId;
	segmentData.info.segmentPath = srcSegmentPath;

	cout << "Segment ID: " << segmentId << " Size: " << formatSize(filesize)
			<< endl;

	// take time for reading segments
	Clock::time_point tRead = Clock::now();

	// perform coding
	vector<BlockData> blockDataList = coding->encode(segmentData,
			codingSetting);

	// take time for coding segments
	Clock::time_point tCode = Clock::now();

	// write segment to storage
	const string segmentPath = segmentFolder + "/" + to_string(segmentId);
	writeFile(segmentPath, segmentData.buf, segmentData.info.segmentSize);

	// write block to storage
	cout << endl << "Writing Blocks: " << endl;
	for (auto block : blockDataList) {
		const string blockPath = blockFolder + "/" + to_string(segmentId) + "."
				+ to_string(block.info.blockId);

		writeFile(blockPath, block.buf, block.info.blockSize);

		cout << block.info.blockId << ": " << blockPath << endl;

		// free blocks
		MemoryPool::getInstance().poolFree(block.buf);
	}

	// free segment
	MemoryPool::getInstance().poolFree(segmentData.buf);

	// take time for writing and clean up
	Clock::time_point tWrite = Clock::now();

	// print timing result
	printResult(tStart, tRead, tCode, tWrite);

	cout << endl << "Command for Decoding: " << endl
			<< "./CODING_TESTER decode " << segmentId << " " << filesize
			<< " ./decoded_file" << endl;

}

void doDecode(uint64_t segmentId, uint64_t segmentSize,
		std::string dstSegmentPath, uint32_t numBlocks,
		vector<bool> blockStatus) {

	// start timer
	Clock::time_point tStart = Clock::now();

	vector<BlockData> blockDataList(numBlocks);

	// find required blocks
	block_list_t requiredBlockSymbols = coding->getRequiredBlockSymbols(
			blockStatus, segmentSize, codingSetting);

	vector<uint32_t> requiredBlocks;
	for (auto blockSymbol : requiredBlockSymbols) {
		requiredBlocks.push_back(blockSymbol.first);
	}

	if (requiredBlocks.size() == 0) {
		cerr << "Not enough blocks to reconstruct file" << endl;
		return;
	}

	// read blocks from files
	cout << "Reading Blocks: " << endl;
	for (uint32_t i : requiredBlocks) {
		const string blockPath = blockFolder + "/" + to_string(segmentId) + "."
				+ to_string(i);

		BlockData blockData;
		uint32_t filesize; // set by reference in readFile
		blockData.buf = readFile(blockPath, filesize); // read block

		// fill in block information
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = filesize;

		cout << i << ": " << blockPath << " size = " << filesize << endl;

		blockDataList[i] = blockData;
	}

	// take time for reading segments
	Clock::time_point tRead = Clock::now();

	// perform decoding
	SegmentData segmentData = coding->decode(blockDataList,
			requiredBlockSymbols, segmentSize, codingSetting);

	// take time for coding segments
	Clock::time_point tCode = Clock::now();

	// write segment to dstSegmentPath
	writeFile(dstSegmentPath, segmentData.buf, segmentData.info.segmentSize);

	// free segment
	MemoryPool::getInstance().poolFree(segmentData.buf);

	// free blocks
	for (auto blockSymbol : requiredBlockSymbols) {
		MemoryPool::getInstance().poolFree(blockDataList[blockSymbol.first].buf);
	}

	// take time for writing and clean up
	Clock::time_point tWrite = Clock::now();

	// print timing result
	printResult(tStart, tRead, tCode, tWrite);

	cout << "Decoded segment written to " << dstSegmentPath << endl;

}

void doRepair(uint64_t segmentId, uint64_t segmentSize, uint32_t numBlocks,
		vector<bool> blockStatus, vector<string> dstBlockPaths) {

	// transform blockStatus to repairBlockList
	vector<uint32_t> repairBlockList;
	for (uint32_t i = 0; i < blockStatus.size(); i++) {
		if (blockStatus[i] == false) {
			repairBlockList.push_back(i);
		}
	}

	// obtain required blockSymbols for repair
	block_list_t blockSymbols = coding->getRepairBlockSymbols(repairBlockList,
			blockStatus, segmentSize, codingSetting);

	if (blockSymbols.size() == 0) {
		cerr << "Not enough blocks to reconstruct file" << endl;
		return;
	}

	vector<BlockData> repairBlockData(
			coding->getBlockCountFromSetting(codingSetting));

	for (auto block : blockSymbols) {

		uint32_t blockId = block.first;

		vector<offset_length_t> offsetLength = block.second;

		const string blockPath = blockFolder + "/" + to_string(segmentId) + "."
				+ to_string(blockId);

		BlockData blockData;
		uint32_t filesize; // set by reference in readFile
		blockData.buf = readFile(blockPath, filesize); // read block

		// fill in block information
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = blockId;
		blockData.info.blockSize = filesize;

		cout << blockId << ": " << blockPath << " size = " << filesize << endl;

		repairBlockData[blockId] = blockData;

	}

	// perform repair
	vector<BlockData> repairedBlocks = coding->repairBlocks(repairBlockList,
			repairBlockData, blockSymbols, segmentSize, codingSetting);

	// write segment to dstBlockPaths
	uint32_t i = 0;
	for (auto repairedblock : repairedBlocks) {
		writeFile(dstBlockPaths[i], repairedblock.buf,
				repairedblock.info.blockSize);
		i++;
	}

	 //free repairBlockData
	for (auto block : blockSymbols) {
		uint32_t blockId = block.first;
		MemoryPool::getInstance().poolFree(repairBlockData[blockId].buf);
	}

	// free repairedBlocks
	for (auto block : repairedBlocks) {
		MemoryPool::getInstance().poolFree(block.buf);
	}

	cout << "Repaired blocks:" << endl;
	for (auto path : dstBlockPaths) {
		cout << path << endl;
	}
}
