#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "raid5coding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid5Coding::Raid5Coding() {

}

Raid5Coding::~Raid5Coding() {

}

vector<struct BlockData> Raid5Coding::encode(struct SegmentData segmentData,
		string setting) {

	vector<struct BlockData> blockDataList;
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t parityIndex = raid5_n - 1; // index starts from 0, last block is parity
	const uint32_t numDataBlock = raid5_n - 1;
	const uint32_t lastDataIndex = raid5_n - 2;

	if (raid5_n < 3) {
		cerr << "At least 3 blocks are needed for RAID-5 encode" << endl;
		exit(-1);
	}

	// calculate size of each data stripe
	const uint32_t stripeSize = Coding::roundTo(segmentData.info.segmentSize,
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

		if (i == lastDataIndex) { // last data block
			blockData.info.blockSize = segmentData.info.segmentSize
					- i * stripeSize;
		} else {
			blockData.info.blockSize = stripeSize;
		}

		// copy data to block
		blockData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);
		char* bufPos = segmentData.buf + i * stripeSize;
		memcpy(blockData.buf, bufPos, blockData.info.blockSize);

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

struct SegmentData Raid5Coding::decode(vector<struct BlockData> &blockData,
		vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
		string setting) {

	if (requiredBlocks.size() < 2) {
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
	segmentData.info.segmentId = blockData[requiredBlocks[0]].info.segmentId;
	segmentData.info.segmentSize = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	struct BlockData rebuildBlockData;

	// if last block of requiredBlocks is the parity block, rebuild is needed
	if (requiredBlocks.back() == parityIndex) {
		rebuildBlockData.buf = MemoryPool::getInstance().poolMalloc(
				stripeSize);

		// rebuild
		uint32_t i = 0;
		for (uint32_t blockId : requiredBlocks) {
			if (i == 0) {
				// memcpy first block
				memcpy(rebuildBlockData.buf, blockData[blockId].buf,
						stripeSize);
			} else {
				// XOR second block onwards
				Coding::bitwiseXor(rebuildBlockData.buf,
						rebuildBlockData.buf, blockData[blockId].buf,
						stripeSize);
			}
			i++;
		}

		// write rebuildBlockData to BlockData
		uint32_t repairedBlockIndex = 0;
		set<uint32_t> requiredBlocksSet(requiredBlocks.begin(),
				requiredBlocks.end()); // convert to set for efficiency

		// find which block to recover
		for (uint32_t i = 0; i < blockData.size(); i++) {
			bool isPresent = requiredBlocksSet.count(i);

			if (!isPresent) {
				repairedBlockIndex = i;

				// fill in rebuildBlockData Information
				rebuildBlockData.info.blockId = repairedBlockIndex;

				if (i == blockData.size() - 1) {
					rebuildBlockData.info.blockSize = segmentSize
							- stripeSize * lastDataIndex;
				} else {
					rebuildBlockData.info.blockSize = stripeSize;
				}

				// block does not exist
				blockData[i] = rebuildBlockData;

				break; // for raid 5, at most one repair
			}
		}

		// free parity block
		MemoryPool::getInstance().poolFree(
				blockData[requiredBlocks.back()].buf);

		// replace parity in requiredBlocks with repaired block and sort
		requiredBlocks.back() = repairedBlockIndex;
		sort(requiredBlocks.begin(), requiredBlocks.end());
	}

	// write block to segment
	uint64_t offset = 0;
	for (uint32_t blockId : requiredBlocks) {
		memcpy(segmentData.buf + offset, blockData[blockId].buf,
				blockData[blockId].info.blockSize);
		offset += blockData[blockId].info.blockSize;
	}

	return segmentData;
}

vector<uint32_t> Raid5Coding::getRequiredBlockIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	if (failedOsdCount > 1) {
		return {};
	}

	// for raid 5, only requires n-1 stripes (raid5_n - 1) to decode
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t noOfDataBlock = raid5_n - 1;
	vector<uint32_t> requiredBlocks;
	requiredBlocks.reserve(noOfDataBlock);

	// no OSD failure / parity OSD failure
	if (failedOsdCount == 0
			|| (failedOsdCount == 1 && secondaryOsdStatus.back() == false)) {
		// select first n-1 blocks
		for (uint32_t i = 0; i < noOfDataBlock; i++) {
			requiredBlocks.push_back(i);
		}
	} else {
		for (uint32_t i = 0; i < raid5_n; i++) {
			// select only available blocks
			if (secondaryOsdStatus[i] != false) {
				requiredBlocks.push_back(i);
			}
		}
	}

	return requiredBlocks;
}

//
// PRIVATE FUNCTION
//

uint32_t Raid5Coding::getParameters(string setting) {
	uint32_t raid5_n;
	istringstream(setting) >> raid5_n;
	return raid5_n;
}

vector<uint32_t> Raid5Coding::getRepairSrcBlockIds(string setting,
		vector<uint32_t> failedBlocks, vector<bool> blockStatus) {

	// at a time only one block can fail
	if (failedBlocks.size() > 1) {
		return {};
	}

	// for RAID5, this is the same case as getRequiredBlockIds
	return getRequiredBlockIds(setting, blockStatus);
}

vector<struct BlockData> Raid5Coding::repairBlocks(
		vector<uint32_t> failedBlocks,
		vector<struct BlockData> &repairSrcBlocks,
		vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
		string setting) {

	const uint32_t raid5_n = getParameters(setting);
	const uint32_t numDataBlock = raid5_n - 1;

	const uint32_t stripeSize = Coding::roundTo(segmentSize, numDataBlock)
			/ numDataBlock;

	struct BlockData rebuildBlockData;

	rebuildBlockData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);

	// rebuild
	uint32_t i = 0;
	for (uint32_t blockId : repairSrcBlockId) {
		if (i == 0) {
			// memcpy first block
			memcpy(rebuildBlockData.buf, repairSrcBlocks[blockId].buf,
					stripeSize);
		} else {
			// XOR second block onwards
			Coding::bitwiseXor(rebuildBlockData.buf, rebuildBlockData.buf,
					repairSrcBlocks[blockId].buf, stripeSize);
		}
		i++;
	}

	return {rebuildBlockData};
}
