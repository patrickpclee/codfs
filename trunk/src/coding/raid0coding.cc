#include <sstream>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <utility>
#include "coding.hh"
#include "raid0coding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Coding::Raid0Coding() {

}

Raid0Coding::~Raid0Coding() {

}

vector<BlockData> Raid0Coding::encode(SegmentData segmentData, string setting) {

	vector<BlockData> blockDataList;
	const uint32_t raid0_n = getParameters(setting);

	// calculate size of each strip
	const uint32_t stripSize = Coding::roundTo(segmentData.info.segmentSize,
			raid0_n) / raid0_n;

	for (uint32_t i = 0; i < raid0_n; i++) {

		BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;

		if (i == raid0_n - 1) { // last block
			blockData.info.blockSize = segmentData.info.segmentSize
					- i * stripSize;

		} else {
			blockData.info.blockSize = stripSize;
		}

		// TODO: free
		blockData.buf = MemoryPool::getInstance().poolMalloc(stripSize);

		char* bufPos = segmentData.buf + i * stripSize;

		memcpy(blockData.buf, bufPos, blockData.info.blockSize);

		blockDataList.push_back(blockData);
	}

	return blockDataList;
}

SegmentData Raid0Coding::decode(vector<BlockData> &blockDataList,
		symbol_list_t &symbolList, uint32_t segmentSize, string setting) {

	// for raid 0, symbolList is not used as all blocks are required to decode

	SegmentData segmentData;

	// copy segmentID from first block
	segmentData.info.segmentId = blockDataList[0].info.segmentId;
	segmentData.info.segmentSize = segmentSize;

	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	uint64_t offset = 0;
	for (BlockData block : blockDataList) {
		memcpy(segmentData.buf + offset, block.buf, block.info.blockSize);
		offset += block.info.blockSize;
	}

	return segmentData;
}

uint32_t Raid0Coding::getParameters(string setting) {
	uint32_t raid0_n;
	istringstream(setting) >> raid0_n;
	return raid0_n;
}

symbol_list_t Raid0Coding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	// if any one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(blockStatus.begin(), blockStatus.end(),
			false);

	if (failedOsdCount > 0) {
		return {};
	}

	// for Raid0 Coding, require all blocks for decode
	const uint32_t raid0_n = getParameters(setting);
	symbol_list_t requiredBlockSymbols(raid0_n);

	for (uint32_t i = 0; i < raid0_n; i++) {
		offset_length_t symbol = make_pair (0, segmentSize);
		vector <offset_length_t> symbolList = {symbol};
		block_symbols_t blockSymbolPair = make_pair (i, symbolList);

		requiredBlockSymbols[i] = blockSymbolPair;
	}
	return requiredBlockSymbols;
}

symbol_list_t Raid0Coding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
			vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	debug_error("%s\n", "Repair not supported in RAID0");

	return {};
}

vector<struct BlockData> Raid0Coding::repairBlocks(
		vector<uint32_t> repairBlockIdList, vector<struct BlockData> &blockData,
		vector<uint32_t> &blockIdList, symbol_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	debug_error("%s\n", "Decode Blocks not supported in RAID0");

	return {};
}
