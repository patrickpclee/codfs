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
		vector<BlockData> &blockDataList, symbol_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	Coding* coding = getCoding(codingScheme);
	struct SegmentData segmentData = coding->decode(blockDataList, symbolList,
			segmentSize, setting);

	return segmentData;
}

symbol_list_t CodingModule::getRequiredBlockSymbols(CodingScheme codingScheme,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {
	return getCoding(codingScheme)->getRequiredBlockSymbols(blockStatus,
			segmentSize, setting);
}

symbol_list_t CodingModule::getRepairBlockSymbols(CodingScheme codingScheme,
		vector<uint32_t> failedBlocks, vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {
	return getCoding(codingScheme)->getRepairBlockSymbols(failedBlocks,
			blockStatus, segmentSize, setting);

}

vector<BlockData> CodingModule::repairBlocks(CodingScheme codingScheme,
		vector<uint32_t> repairBlockIdList, vector<BlockData> &blockData,
		vector<uint32_t> &blockIdList, symbol_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	return getCoding(codingScheme)->repairBlocks(repairBlockIdList, blockData,
			blockIdList, symbolList, segmentSize, setting);

}

Coding* CodingModule::getCoding(CodingScheme codingScheme) {

	lock_guard<mutex> lk(codingMutex);

	if (!_codingWorker.count(codingScheme)) {
		debug_error("%s\n", "Wrong coding scheme!");
		exit(-1);
	}

	return _codingWorker[codingScheme];
}
