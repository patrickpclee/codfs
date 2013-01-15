#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "evenoddcoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

EvenOddCoding::EvenOddCoding() {

}

EvenOddCoding::~EvenOddCoding() {

}

vector<BlockData> EvenOddCoding::encode(SegmentData segmentData, string setting) {
	const uint32_t k = getBlockCountFromSetting(setting) - 2;
	const uint32_t blockSize = roundTo(segmentData.info.segmentSize, k * (k - 1)) / k;
	const uint32_t symbolSize = blockSize / (k - 1);

	vector<struct BlockData> blockDataList;
	blockDataList.reserve(k + 2);

	struct BlockData blockData;
	blockData.info.segmentId = segmentData.info.segmentId;
	blockData.info.blockSize = blockSize;
	for(uint32_t i = k; i < k + 2; ++i) {
		blockData.info.blockId = i;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);		
		blockDataList[i] = blockData;
	}

	uint32_t d_group;
	for(uint32_t i = 0; i < k; ++i) {
		d_group = i;
		blockData.info.blockId = i;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);		

		// Copy Data
		char* bufPos = segmentData.buf + i * blockSize;
		if (i == k - 1) {
			//memset(blockData.buf, 0, size);
			memcpy(blockData.buf, bufPos,
					segmentData.info.segmentSize - i * blockSize);
			memset(blockData.buf + segmentData.info.segmentSize - i * blockSize, 0,
					k * blockSize - segmentData.info.segmentSize);
		} else
			memcpy(blockData.buf, bufPos, blockSize);

		// Compute Row Parity Block
		if (i == 0) {
			memcpy(blockDataList[k].buf, blockData.buf, blockSize);
		} else {
			bitwiseXor(blockDataList[k].buf, blockDataList[k].buf, blockData.buf, blockSize);
		}

		// Compute Diagonal Parity Block
		for(uint32_t j = 0; j < k - 1; ++j) {		
			if(d_group == k - 1) {
				for(uint32_t l = 0; l < k - 1; ++l) {
					bitwiseXor(blockDataList[k + 1].buf + l * symbolSize, blockDataList[k + 1].buf + l * symbolSize, blockData.buf + j * symbolSize, symbolSize);
				}
			} else
				bitwiseXor(blockDataList[k + 1].buf + d_group * symbolSize, blockDataList[k + 1].buf + d_group * symbolSize, blockData.buf + j * symbolSize, symbolSize);

			d_group = (d_group + 1) % k;
		}


		blockDataList[i] = blockData;
	}

	return blockDataList;
}

SegmentData EvenOddCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	SegmentData segmentData;

	return segmentData;
}

block_list_t EvenOddCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	const uint32_t k = getBlockCountFromSetting(setting) - 2;
	const uint32_t blockSize = roundTo(segmentSize, k * (k - 1)) / k;
	const uint32_t symbolSize = blockSize / (k - 1);
	const uint32_t numOfFailed = (uint32_t)std::count(blockStatus.begin(), blockStatus.end(), false);
	uint32_t row_group_erasure[k - 1];
	uint32_t diagonal_group_erasure[k - 1];
	std::fill_n (row_group_erasure, k - 1, numOfFailed);
	std::fill_n (diagonal_group_erasure, k - 1, numOfFailed);

	block_list_t requiredBlockList;
	return requiredBlockList;
}

block_list_t EvenOddCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	block_list_t requiredBlockList;
	return requiredBlockList;
}

vector<BlockData> EvenOddCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	return {};
}

uint32_t EvenOddCoding::getBlockCountFromSetting(string setting) {
	return getParameters(setting);
}

//
// PRIVATE FUNCTION
//

uint32_t EvenOddCoding::getParameters(string setting) {
	uint32_t n;
	istringstream(setting) >> n;
	return n;
}
