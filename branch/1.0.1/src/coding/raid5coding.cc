#include <sstream>
#include <iostream>
#include <algorithm>
#include <openssl/md5.h>
#include <set>
#include <string.h>
#include "coding.hh"
#include "raid5coding.hh"
#include "../common/debug.hh"
#include "../common/convertor.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid5Coding::Raid5Coding() {

}

Raid5Coding::~Raid5Coding() {

}

vector<BlockData> Raid5Coding::encode(SegmentData segmentData, string setting) {

	vector<struct BlockData> blockDataList;
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t parityIndex = raid5_n - 1; // index starts from 0, last block is parity
	const uint32_t numDataBlock = raid5_n - 1;

	if (raid5_n < 3) {
		cerr << "At least 3 blocks are needed for RAID-5 encode" << endl;
		exit(-1);
	}

	// calculate size of each data stripe
	const uint32_t stripeSize = Coding::roundTo(segmentData.info.segLength,
			numDataBlock) / numDataBlock;

	// prepare parity block
	struct BlockData parityBlockData;
	parityBlockData.info.segmentId = segmentData.info.segmentId;
	parityBlockData.info.blockId = parityIndex;
	parityBlockData.info.blockSize = stripeSize;
	parityBlockData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);

	// for each data block
	for (uint32_t i = 0; i < parityIndex; i++) {

		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = stripeSize;

		// copy data to block
		blockData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);
		char* bufPos = segmentData.buf + i * stripeSize;
		if (i * stripeSize >= segmentData.info.segLength) {
			//Zero Padding
		} else if ((i + 1) * stripeSize > segmentData.info.segLength) {
			memcpy(blockData.buf, bufPos,
					segmentData.info.segLength - i * stripeSize);
			memset(blockData.buf + segmentData.info.segLength - i * stripeSize, 0,
					(i + 1) * stripeSize - segmentData.info.segLength);
		} else
			memcpy(blockData.buf, bufPos, stripeSize);

		blockDataList.push_back(blockData);

		// update parity block
		if (i == 0) {
			// first block: just copy
			memcpy(parityBlockData.buf, blockData.buf, stripeSize);
		} else {
			// for second block onwards, do XOR
			Coding::bitwiseXor(parityBlockData.buf, parityBlockData.buf,
					blockData.buf, stripeSize);
		}
	}

	// add parity block at the back
	blockDataList.push_back(parityBlockData);

	return blockDataList;
}

SegmentData Raid5Coding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	if (symbolList.size() < 2) {
		cerr << "At least 2 blocks are needed for RAID-5 decode" << endl;
		exit(-1);
	}

	struct SegmentData segmentData;
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t parityIndex = raid5_n - 1; // index starts from 0, last block is parity
	const uint32_t numDataBlock = raid5_n - 1;
	const uint32_t lastDataIndex = raid5_n - 2;

	const uint32_t stripeSize = Coding::roundTo(segmentSize, numDataBlock)
			/ numDataBlock;

	// copy segmentID from first available block
	segmentData.info.segmentId =
			blockDataList[symbolList[0].first].info.segmentId;
	segmentData.info.segLength = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	struct BlockData rebuildBlockData;

	// if last block of blockIdList is the parity block, rebuild is needed
	if (symbolList.back().first == parityIndex) {
		rebuildBlockData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);

		// rebuild
		uint32_t i = 0;
		for (auto blockSymbols : symbolList) {
			uint32_t blockId = blockSymbols.first;
			if (i == 0) {
				// memcpy first block
				memcpy(rebuildBlockData.buf, blockDataList[blockId].buf,
						stripeSize);
			} else {
				// XOR second block onwards
				Coding::bitwiseXor(rebuildBlockData.buf, rebuildBlockData.buf,
						blockDataList[blockId].buf, stripeSize);
			}
			i++;
		}

		// write rebuildBlockData to BlockData
		uint32_t repairedBlockIndex = 0;

		// convert to set for efficiency
		set<uint32_t> requiredBlocksSet;
		for (auto blockSymbols : symbolList) {
			requiredBlocksSet.insert(blockSymbols.first);
		}

		// find which block to recover
		for (uint32_t i = 0; i < blockDataList.size(); i++) {
			bool isPresent = requiredBlocksSet.count(i);

			if (!isPresent) {
				repairedBlockIndex = i;

				// fill in rebuildBlockData Information
				rebuildBlockData.info.blockId = repairedBlockIndex;
				rebuildBlockData.info.blockSize = stripeSize;

				// block at i does not exist
				blockDataList[i] = rebuildBlockData;

				break; // for raid 5, at most one repair
			}
		}

		// free parity block
		MemoryPool::getInstance().poolFree(
				blockDataList[parityIndex].buf);

		// replace parity in symbolList with repaired block and sort
		symbolList.back().first = repairedBlockIndex;
        symbolList.back().second = {make_pair(0, rebuildBlockData.info.blockSize)};
		sort(symbolList.begin(), symbolList.end());
	}

	// write block to segment
	uint64_t offset = 0;
	for (auto blockSymbols : symbolList) {
		uint32_t blockId = blockSymbols.first;
		uint32_t copySize = 0;
		if (blockId == lastDataIndex) {
			copySize = segmentSize - lastDataIndex * stripeSize;
		} else {
			copySize = stripeSize;
		}
		memcpy(segmentData.buf + offset, blockDataList[blockId].buf,
				copySize);
		offset += copySize;
	}

	return segmentData;
}

block_list_t Raid5Coding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(blockStatus.begin(), blockStatus.end(),
			false);

	if (failedOsdCount > 1) {
		return {};
	}

	// for raid 5, only requires n-1 stripes (raid5_n - 1) to decode
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t dataBlockCount = raid5_n - 1;
	block_list_t requiredBlockSymbols;
	requiredBlockSymbols.reserve(dataBlockCount);

	// no OSD failure / parity OSD failure
	uint32_t blockRange = 0;

	if (failedOsdCount == 0
			|| (failedOsdCount == 1 && blockStatus.back() == false)) {
		blockRange = dataBlockCount; // select first n-1 blocks
	} else {
		blockRange = raid5_n;
	}

	uint32_t blockSize = roundTo(segmentSize, dataBlockCount) / dataBlockCount;

	for (uint32_t i = 0; i < blockRange; i++) {
		// select only available blocks
		if (blockStatus[i] != false) {
			offset_length_t symbol = make_pair(0, blockSize);
			vector<offset_length_t> symbolList = { symbol };
			symbol_list_t blockSymbols = make_pair(i, symbolList);
			requiredBlockSymbols.push_back(blockSymbols);
		}
	}

	return requiredBlockSymbols;

}

//
// PRIVATE FUNCTION
//

uint32_t Raid5Coding::getParameters(string setting) {
	uint32_t raid5_n;
	istringstream(setting) >> raid5_n;
	return raid5_n;
}

block_list_t Raid5Coding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	// at a time only one block can fail
	if (failedBlocks.size() > 1) {
		return {};
	}

	// for RAID5, this is the same case as getRequiredBlockIds
	return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
}

vector<BlockData> Raid5Coding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	if (repairBlockIdList.size() > 1) {
		debug_error(
				"Raid5 only supports one rebuild max, rebuild size = %zu\n",
				repairBlockIdList.size());
	}

	const uint32_t raid5_n = getParameters(setting);
	const uint32_t dataBlockCount = raid5_n - 1;

	const uint32_t blockSize = Coding::roundTo(segmentSize, dataBlockCount)
			/ dataBlockCount;

	struct BlockData rebuildBlockData;

	rebuildBlockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);

	// rebuild
	uint32_t i = 0;
	for (auto block : symbolList) {
		uint32_t blockId = block.first;
		if (i == 0) {
			// memcpy first block
			memcpy(rebuildBlockData.buf, blockData[blockId].buf, blockSize);
		} else {
			// XOR second block onwards
			Coding::bitwiseXor(rebuildBlockData.buf, rebuildBlockData.buf,
					blockData[blockId].buf, blockSize);
		}
		i++;
	}

	rebuildBlockData.info.blockId = repairBlockIdList[0];
	rebuildBlockData.info.segmentId = blockData[symbolList[0].first].info.segmentId;

	rebuildBlockData.info.blockSize = blockSize;

	blockData[repairBlockIdList[0]] = rebuildBlockData;

	// for raid5, there can be only one rebuildBlockData
	return {rebuildBlockData};
}

uint32_t Raid5Coding::getBlockCountFromSetting (string setting) {
	return getParameters(setting);
}

uint32_t Raid5Coding::getParityCountFromSetting (string setting) {
	return 1;
}

uint32_t Raid5Coding::getBlockSize(uint32_t segmentSize, string setting) {
	uint32_t n = getParameters(setting);
	return roundTo(segmentSize, (n - 1)) / (n - 1);
}
