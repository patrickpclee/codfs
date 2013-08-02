#include <sstream>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "../common/debug.hh"
#include "coding.hh"
#include "raid1coding.hh"
#include "../common/memorypool.hh"

Raid1Coding::Raid1Coding() {

}

Raid1Coding::~Raid1Coding() {

}

vector<BlockData> Raid1Coding::encode(SegmentData segmentData, string setting) {

	const uint32_t raid1_n = getParameters(setting);
	vector<struct BlockData> blockDataList;

	debug("RAID1: Replication No = %" PRIu32 "\n", raid1_n);

	for (uint32_t i = 0; i < raid1_n; i++) {

		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = segmentData.info.segLength;

		// an optimization is to point the buf pointer to the same memory,
		// but it may create confusion when user wants to free the data

		blockData.buf = MemoryPool::getInstance().poolMalloc(
				segmentData.info.segLength);

		memcpy(blockData.buf, segmentData.buf, blockData.info.blockSize);

		blockDataList.push_back(blockData);
	}

	return blockDataList;
}

SegmentData Raid1Coding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	// for raid1, only use first required block to decode
	uint32_t blockId = symbolList[0].first;

	struct SegmentData segmentData;
	segmentData.info.segmentId = blockDataList[blockId].info.segmentId;
	segmentData.info.segLength = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(
			segmentData.info.segLength);
	memcpy(segmentData.buf, blockDataList[blockId].buf,
			segmentData.info.segLength);

	return segmentData;
}

uint32_t Raid1Coding::getParameters(string setting) {
	uint32_t raid1_n;
	istringstream(setting) >> raid1_n;
	return raid1_n;
}

block_list_t Raid1Coding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	// for Raid1 Coding, find the first running OSD
	vector<bool>::iterator it;
	it = find(blockStatus.begin(), blockStatus.end(), true);

	// not found (no OSD is running)
	if (it == blockStatus.end()) {
		return {};
	}

	// return the index
	uint32_t offset = it - blockStatus.begin();
	offset_length_t symbol = make_pair(0, segmentSize);
	vector<offset_length_t> symbolList = { symbol };
	symbol_list_t blockSymbols = make_pair(offset, symbolList);
	return {blockSymbols};
}

block_list_t Raid1Coding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	// for RAID-1, repair is same as normal download
	return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
}

vector<BlockData> Raid1Coding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	// for raid1, only use first required block to decode
	vector<BlockData> repairedBlockDataList;
	repairedBlockDataList.reserve(repairBlockIdList.size());

	for (uint32_t blockId : repairBlockIdList) {
		BlockData srcBlock = blockData[symbolList[0].first];
		BlockData repairedBlock;

		uint32_t blockSize = srcBlock.info.blockSize;
		repairedBlock.info.segmentId = srcBlock.info.segmentId;
		repairedBlock.info.blockId = blockId;
		repairedBlock.info.blockSize = blockSize;
		repairedBlock.buf = MemoryPool::getInstance().poolMalloc(blockSize);
		memcpy(repairedBlock.buf, srcBlock.buf, blockSize);
		repairedBlockDataList.push_back(repairedBlock);
	}

	return repairedBlockDataList;
}

uint32_t Raid1Coding::getBlockCountFromSetting (string setting) {
	return getParameters(setting);
}

uint32_t Raid1Coding::getBlockSize(uint32_t segmentSize, string setting) {
	return segmentSize;
}
