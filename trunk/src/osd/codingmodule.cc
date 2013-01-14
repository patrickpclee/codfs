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
#include "../coding/evenoddcoding.hh"
#include "../coding/rscoding.hh"
#include "../coding/embrcoding.hh"
#include "../common/debug.hh"

mutex codingMutex;

CodingModule::CodingModule() {

	{
		lock_guard<mutex> lk(codingMutex);
		_codingWorker[RAID0_CODING] = new Raid0Coding();
		_codingWorker[RAID1_CODING] = new Raid1Coding();
		_codingWorker[RAID5_CODING] = new Raid5Coding();
		_codingWorker[EVENODD_CODING] = new EvenOddCoding();
		_codingWorker[RS_CODING] = new RSCoding();
		_codingWorker[EMBR_CODING] = new EMBRCoding();
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
		vector<BlockData> &blockDataList, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	Coding* coding = getCoding(codingScheme);
	struct SegmentData segmentData = coding->decode(blockDataList, symbolList,
			segmentSize, setting);

	return segmentData;
}

block_list_t CodingModule::getRequiredBlockSymbols(CodingScheme codingScheme,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {
	return getCoding(codingScheme)->getRequiredBlockSymbols(blockStatus,
			segmentSize, setting);
}

block_list_t CodingModule::getRepairBlockSymbols(CodingScheme codingScheme,
		vector<uint32_t> failedBlocks, vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	block_list_t blockSymbols = getCoding(codingScheme)->getRepairBlockSymbols(failedBlocks,
			blockStatus, segmentSize, setting);

	for (auto block : blockSymbols) {
		debug_cyan ("[RECOVERY(CODING)] symbol %" PRIu32 "\n", block.first);
	}
	return blockSymbols;
}

vector<BlockData> CodingModule::repairBlocks(CodingScheme codingScheme,
		vector<uint32_t> repairBlockIdList, vector<BlockData> &blockData,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	return getCoding(codingScheme)->repairBlocks(repairBlockIdList, blockData,
			symbolList, segmentSize, setting);

}

Coding* CodingModule::getCoding(CodingScheme codingScheme) {

	lock_guard<mutex> lk(codingMutex);

	if (!_codingWorker.count(codingScheme)) {
		debug_error("%s\n", "Wrong coding scheme!");
		exit(-1);
	}

	return _codingWorker[codingScheme];
}

uint32_t CodingModule::getNumberOfBlocks(CodingScheme codingScheme, string setting) {
	return getCoding(codingScheme)->getBlockCountFromSetting(setting);
}
