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
#include "../coding/embrcoding.hh"
#include "../coding/evenoddcoding.hh"
#include "../coding/rdpcoding.hh"
#include "../common/debug.hh"

mutex codingMutex;

CodingModule::CodingModule() {

	{
		lock_guard<mutex> lk(codingMutex);
		_codingWorker[RAID0_CODING] = new Raid0Coding();
		_codingWorker[RAID1_CODING] = new Raid1Coding();
		_codingWorker[RAID5_CODING] = new Raid5Coding();
		_codingWorker[RS_CODING] = new RSCoding();
		_codingWorker[EMBR_CODING] = new EMBRCoding();
		_codingWorker[EVENODD_CODING] = new EvenOddCoding();
		_codingWorker[RDP_CODING] = new RDPCoding();
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
	segmentData.info.segLength = length;
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

uint32_t CodingModule::getParityNumber(CodingScheme codingScheme, string setting) {
	return getCoding(codingScheme)->getParityCountFromSetting(setting);
}

uint32_t CodingModule::getBlockSize(CodingScheme codingScheme, string setting, 
                                    uint32_t segmentSize) {
	return getCoding(codingScheme)->getBlockSize(segmentSize, setting);
}

BlockData CodingModule::computeDelta(CodingScheme codingScheme, string setting, BlockData oldBlock, BlockData newBlock,
        vector<offset_length_t> offsetLength) {
    return getCoding(codingScheme)->computeDelta(oldBlock, newBlock, offsetLength);
}

vector<BlockData> CodingModule::unpackUpdates(CodingScheme codingScheme, 
        uint64_t segmentId, char* segmentBuf, uint32_t segmentSize, string setting, 
        vector<offset_length_t> offlenVector) {

    vector<BlockData> blkDataList;
    uint32_t blkSize = getCoding(codingScheme)->getBlockSize(segmentSize, setting);
    uint32_t blkN = getCoding(codingScheme)->getBlockCountFromSetting(setting);
    uint32_t blkBufSize[blkN];
    vector<offset_length_t> offlenVectors[blkN];

    blkDataList.clear();
    for (uint32_t i = 0; i < blkN; i++) offlenVectors[i].clear();
    memset(blkBufSize, 0, sizeof(blkBufSize));

    // offlenVecotr should be sorted

    for (auto& offlen: offlenVector) {
        uint32_t curHead = offlen.first;
        uint32_t curTail = offlen.first + offlen.second;
        uint32_t headIdx = curHead / blkSize;
        uint32_t tailIdx = (curTail-1) / blkSize;
        while (headIdx != tailIdx) {
            // cut to current blk size boundary
            uint32_t curSize = blkSize * (headIdx + 1) - curHead;
            // save to tmp
            blkBufSize[headIdx] += curSize;
            offlenVectors[headIdx].push_back(make_pair(curHead-headIdx*blkSize, curSize));
            //update idx
            curHead = blkSize * (headIdx + 1);
            headIdx = curHead / blkSize;
        }
        blkBufSize[tailIdx] += curTail - curHead;
        offlenVectors[tailIdx].push_back(make_pair(curHead-headIdx*blkSize, curTail-curHead));
    }

    uint32_t toffset = 0;
    for (uint32_t i = 0; i < blkN; i++) {
        if (blkBufSize[i] != 0) {
            // contains update in this blk
            BlockData blkData;
            blkData.info.segmentId = segmentId;
            blkData.info.blockId = i;
            blkData.info.blockSize = blkBufSize[i];
            blkData.info.offlenVector.swap(offlenVectors[i]);

	        blkData.buf = MemoryPool::getInstance().poolMalloc(blkBufSize[i]);
            memcpy(blkData.buf, segmentBuf + toffset, blkBufSize[i]);
            toffset += blkBufSize[i];

            blkDataList.push_back(blkData);
        }
    }

    return blkDataList;
}
