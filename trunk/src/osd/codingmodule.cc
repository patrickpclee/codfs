/*
 * codingmodule.cc
 */

#include <thread>
#include <mutex>
#include <iostream>
using namespace std;
#include "codingmodule.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"
#include "../common/debug.hh"

mutex codingMutex;

CodingModule::CodingModule() {

	{
		lock_guard<mutex> lk(codingMutex);
		_codingWorker[RAID0_CODING] = new Raid0Coding();
		_codingWorker[RAID1_CODING] = new Raid1Coding();
		_codingWorker[RAID5_CODING] = new Raid5Coding();
		_codingWorker[RS_CODING] = new RSCoding();
	}
}

vector<struct BlockData> CodingModule::encodeSegmentToBlock(
		CodingScheme codingScheme, struct SegmentData segmentData,
		string setting) {

	return getCoding(codingScheme)->encode(segmentData, setting);
}

vector<struct BlockData> CodingModule::encodeSegmentToBlock(
		CodingScheme codingScheme, uint64_t segmentId, char* buf,
		uint64_t length, string setting) {

	struct SegmentData segmentData;
	segmentData.buf = buf;
	segmentData.info.segmentId = segmentId;
	segmentData.info.segmentSize = length;
	return getCoding(codingScheme)->encode(segmentData, setting);
}

struct SegmentData CodingModule::decodeBlockToSegment(CodingScheme codingScheme,
		uint64_t segmentId, vector<struct BlockData> &blockData,
		vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
		string setting) {

	Coding* coding = getCoding(codingScheme);
	struct SegmentData segmentData = coding->decode(blockData, requiredBlocks,
			segmentSize, setting);

	return segmentData;
}

Coding* CodingModule::getCoding(CodingScheme codingScheme) {

	lock_guard<mutex> lk(codingMutex);

	if (!_codingWorker.count(codingScheme)) {
		debug_error("%s\n", "Wrong coding scheme!");
		exit(-1);
	}

	return _codingWorker[codingScheme];
}

vector<uint32_t> CodingModule::getRequiredBlockIds(CodingScheme codingScheme,
		string setting, vector<bool> secondaryOsdStatus) {
	return getCoding(codingScheme)->getRequiredBlockIds(setting,
			secondaryOsdStatus);
}

vector<uint32_t> CodingModule::getRepairSrcBlockIds(CodingScheme codingScheme,
		string setting, vector<uint32_t> failedBlocks,
		vector<bool> blockStatus) {
	return getCoding(codingScheme)->getRepairSrcBlockIds(setting,
			failedBlocks, blockStatus);

}

vector<struct BlockData> CodingModule::repairBlocks(
		CodingScheme codingScheme, vector<uint32_t> failedBlocks,
		vector<struct BlockData> &repairSrcBlocks,
		vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
		string setting) {

	return getCoding(codingScheme)->repairBlocks(failedBlocks,
			repairSrcBlocks, repairSrcBlockId, segmentSize, setting);

}
