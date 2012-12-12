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

vector<struct BlockData> Raid1Coding::encode(struct SegmentData segmentData,
		string setting) {

	const uint32_t raid1_n = getParameters(setting);
	vector<struct BlockData> blockDataList;

	debug("RAID1: Replication No = %" PRIu32 "\n", raid1_n);

	for (uint32_t i = 0; i < raid1_n; i++) {

		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = segmentData.info.segmentSize;

		// an optimization is to point the buf pointer to the same memory,
		// but it may create confusion when user wants to free the data

		blockData.buf = MemoryPool::getInstance().poolMalloc(
				segmentData.info.segmentSize);

		memcpy(blockData.buf, segmentData.buf, blockData.info.blockSize);

		blockDataList.push_back(blockData);
	}

	return blockDataList;
}

struct SegmentData Raid1Coding::decode(vector<struct BlockData> &blockData,
		vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
		string setting) {

	// for raid1, only use first required block to decode
	uint32_t blockId = requiredBlocks[0];

	struct SegmentData segmentData;
	segmentData.info.segmentId = blockData[blockId].info.segmentId;
	segmentData.info.segmentSize = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(
			segmentData.info.segmentSize);
	memcpy(segmentData.buf, blockData[blockId].buf,
			segmentData.info.segmentSize);

	return segmentData;
}

uint32_t Raid1Coding::getParameters(string setting) {
	uint32_t raid1_n;
	istringstream(setting) >> raid1_n;
	return raid1_n;
}

vector<uint32_t> Raid1Coding::getRequiredBlockIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// for Raid1 Coding, find the first running OSD
	vector<bool>::iterator it;
	it = find(secondaryOsdStatus.begin(), secondaryOsdStatus.end(), true);

	// not found (no OSD is running)
	if (it == secondaryOsdStatus.end()) {
		return {};
	}

	// return the index
	uint32_t offset = it - secondaryOsdStatus.begin();
	return {offset};
}

vector<uint32_t> Raid1Coding::getRepairSrcBlockIds(string setting,
		vector<uint32_t> failedBlocks, vector<bool> blockStatus) {

	// for Raid1 Coding, find the first running OSD
	vector<bool>::iterator it;
	it = find(blockStatus.begin(), blockStatus.end(), true);

	// not found (no OSD is running)
	if (it == blockStatus.end()) {
		debug_error("%s\n", "No more copies available to repair");
		return {};
	}

	// return the index
	uint32_t offset = it - blockStatus.begin();
	return {offset};
}

vector<struct BlockData> Raid1Coding::repairBlocks(
		vector<uint32_t> failedBlocks,
		vector<struct BlockData> &repairSrcBlocks,
		vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
		string setting){

	// for raid1, only use first required block to decode
	vector<BlockData> blockDataList;
	blockDataList.reserve(failedBlocks.size());

	for (uint32_t blockId : failedBlocks) {

		struct BlockData blockData;
		blockData.info.segmentId = repairSrcBlocks[repairSrcBlockId[0]].info.segmentId;
		blockData.info.blockId = blockId;
		blockData.info.blockSize = repairSrcBlocks[repairSrcBlockId[0]].info.blockSize;
		blockData.buf = MemoryPool::getInstance().poolMalloc(
				blockData.info.blockSize);
		memcpy(blockData.buf, repairSrcBlocks[repairSrcBlockId[0]].buf,
				blockData.info.blockSize);
		blockDataList.push_back(blockData);
	}

	return blockDataList;
}
