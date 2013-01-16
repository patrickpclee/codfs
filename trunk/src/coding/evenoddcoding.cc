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
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = roundTo(segmentData.info.segmentSize, k * (k - 1)) / k;
	const uint32_t symbolSize = blockSize / (k - 1);
	
	vector<struct BlockData> blockDataList;
	blockDataList.reserve(n);

	struct BlockData blockData;
	blockData.info.segmentId = segmentData.info.segmentId;
	blockData.info.blockSize = blockSize;
	for(uint32_t i = 0; i < n; ++i)
		blockDataList.push_back(blockData);

	for(uint32_t i = k; i < n; ++i) {
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

	debug("Block Data List Count: %" PRIu32 "\n", blockDataList.size());
	return blockDataList;
}

SegmentData EvenOddCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &blockList, uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = roundTo(segmentSize, k * (k - 1)) / k;
	const uint32_t symbolSize = blockSize / (k - 1);
	uint32_t row_group_erasure[k - 1];
	uint32_t diagonal_group_erasure[k];
	uint32_t datadisk_block_erasure[k];
	bool disk_block_status[n][k - 1];
	bool need_diagonal_adjust[k][k - 1];
	uint32_t numOfFailedDataDisk = k;
	std::fill_n (row_group_erasure, k - 1, k + 1);
	std::fill_n (diagonal_group_erasure, k - 1, k);
	std::fill_n (datadisk_block_erasure, k, k - 1);
	char* diagonal_parity = MemoryPool::getInstance().poolMalloc(symbolSize);
	char* repair_symbol = MemoryPool::getInstance().poolMalloc(symbolSize);

	bool use_diagonal = false;
	bool use_row = false;

	char* tempBlock[n];
	for(uint32_t i = 0; i < n; ++i) {
		tempBlock[i] = MemoryPool::getInstance().poolMalloc(blockSize);
		std::fill_n(disk_block_status[i], k - 1, false);
		if(i < k)
			std::fill_n(need_diagonal_adjust[i], k - 1, false);
	}
	diagonal_group_erasure[k - 1] = k - 1;

	for(auto blockSymbols: blockList) {
		uint32_t id = blockSymbols.first;
		if(id == k + 1)
			use_diagonal = true;
		else if (id == k)
			use_row = true;
	}

	for(auto blockSymbols: blockList) {
		vector<offset_length_t> symbolList = blockSymbols.second;
		uint32_t id = blockSymbols.first;
		if(id < k) { 
			datadisk_block_erasure[id] = 0;
			--numOfFailedDataDisk;
		}
		uint32_t offset = 0;
		for(auto symbol : symbolList) {
			memcpy(tempBlock[id] + symbol.first, blockDataList[id].buf + offset, symbol.second);
			offset += symbol.second;
			uint32_t firstSymbolId = symbol.first / symbolSize;
			for(uint32_t i = 0; i < (symbol.second / symbolSize); ++i) {
				disk_block_status[id][firstSymbolId + i] = true;
				if(use_row && (id != k + 1)) {
					uint32_t row_group = firstSymbolId + i;
					--row_group_erasure[row_group];
				}
				if(use_diagonal && (id != k)) {
					uint32_t diagonal_group = (firstSymbolId + i + id) % k;
					if(id == k + 1) {
						--diagonal_group;
					}
					--diagonal_group_erasure[diagonal_group];
					debug("%" PRIu32 ":%" PRIu32 " Diagonal Group %" PRIu32 "\n", id, firstSymbolId + i, diagonal_group);
					if(diagonal_group == k - 1) 
						bitwiseXor(diagonal_parity, tempBlock[id] + i * symbolSize, diagonal_parity, symbolSize);	
				}
			}
		}
	}


	bool diagonal_parity_failed = !(diagonal_group_erasure[k - 1] == 0);
	bool diagonal_clean[k];
	std::fill_n(diagonal_clean, k , true);
	vector<block_symbol_t> needDiagonalFix = {};

	// Fix Failed Data Disk
	uint32_t id;
	uint32_t symbol;
	uint32_t target_id;
	uint32_t target_symbol;
	while(numOfFailedDataDisk > 0) {
		for(uint32_t i = 0; i < k - 1; ++i){
			debug("Row %" PRIu32 ": %" PRIu32 " Erasures, Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", i, row_group_erasure[i], i, diagonal_group_erasure[i]);
		}
		debug("Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", k - 1, diagonal_group_erasure[k - 1]);
		target_id = (uint32_t)-1;
		target_symbol = (uint32_t)-1;
		memset(repair_symbol, 0, symbolSize);
		bool cur_symbol_need_adjust = false;
		for(uint32_t i = 0; i < k - 1; ++i) {
			// Fix by Row
			if(use_row && (row_group_erasure[i] == 1)) {
				cur_symbol_need_adjust = false;
				symbol = i;
				target_symbol = i;
				for(uint32_t j = 0; j < k + 1; ++j) {
					id = j;
					if((id < k) && (disk_block_status[id][symbol] == false))
						target_id = id;
					else {
						bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
						if(need_diagonal_adjust[id][symbol]){
							debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
							cur_symbol_need_adjust = !cur_symbol_need_adjust;
						}
					}
				}
				
				if(use_diagonal) {
					uint32_t diagonal_group = (target_symbol + target_id) % k;
					// Fix Diagonal Parity Adjuster
					if(diagonal_group == k - 1)
						bitwiseXor(diagonal_parity, repair_symbol, diagonal_parity, symbolSize);
					--diagonal_group_erasure[diagonal_group];
					diagonal_clean[diagonal_group] = diagonal_clean[diagonal_group] != cur_symbol_need_adjust;
				}

				--row_group_erasure[symbol];

				debug("Fix %" PRIu32 ":%" PRIu32 " with row %" PRIu32 "\n", target_id, target_symbol, symbol);
				break;
			}

			// Fix by Diagonal
			if(use_diagonal && (diagonal_group_erasure[i] == 1)) {
				cur_symbol_need_adjust = true;
				for(uint32_t j = 0; j < k - 1; ++j){
					id = (i - j + k) % k;
					symbol = (i - id + k) % k;
					if((id < k) && (disk_block_status[id][symbol] == false)) {
						target_id = id;
						target_symbol = symbol;
					} else {
						bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
						if(need_diagonal_adjust[id][symbol]){
							debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
							cur_symbol_need_adjust = !cur_symbol_need_adjust;
						}
					}
				}
				bitwiseXor(repair_symbol, repair_symbol, tempBlock[k + 1] + i * symbolSize, symbolSize);

				diagonal_clean[i] = diagonal_clean[i] != cur_symbol_need_adjust;
				
				diagonal_group_erasure[i] = 0;
				if(use_row)
					--row_group_erasure[target_symbol];
				debug("Fix %" PRIu32 ":%" PRIu32 " with diagonal %" PRIu32 "\n", target_id, target_symbol, i);
				break;
			}
		}

		// Fix Diagonal Adjuster
		if(use_diagonal && (target_id == (uint32_t)-1) && (diagonal_group_erasure[k - 1] == 1)) {
			cur_symbol_need_adjust = true;
			id = k - 1;
			symbol = 0;
			for(uint32_t i = 0; i < k - 1; ++i) {
				if(disk_block_status[id][symbol]){
					bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
					if(need_diagonal_adjust[id][symbol]){
						debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
						cur_symbol_need_adjust = !cur_symbol_need_adjust;
					}
				} else {
					target_id = id;
					target_symbol = symbol;
				}
				--id;
				++symbol;
			}
		}

		if(target_id == (uint32_t)-1){
			debug_error("%s\n","Cannot fix any");
			exit(-1);
		}

		if(cur_symbol_need_adjust) {
			debug("%" PRIu32 ":%" PRIu32 " need diagonal adjustment\n", target_id, target_symbol);
			need_diagonal_adjust[target_id][target_symbol] = true;
			needDiagonalFix.push_back(make_pair(target_id, target_symbol));
		}

		disk_block_status[target_id][target_symbol] = true;

		memcpy(tempBlock[target_id] + target_symbol * symbolSize, repair_symbol, symbolSize);
		
		--datadisk_block_erasure[target_id];
		debug("Disk %" PRIu32 " Erasure %" PRIu32 "\n", target_id, datadisk_block_erasure[target_id]);
		if(datadisk_block_erasure[target_id] == 0){
			--numOfFailedDataDisk;
			debug("Disk %" PRIu32 " Fixed, %" PRIu32 " Failure Left\n", target_id, numOfFailedDataDisk);
		}
	}

	if(use_diagonal) {
		// Find a Clean Diagonal Parity
		//if(diagonal_parity_failed && !diagonal_clean[k - 1]) {
		if(diagonal_parity_failed){
			for(uint32_t i = 0; i < k - 1; ++i)
				if(diagonal_clean[i]){
					debug("Clean Diagonal Found at %" PRIu32 "\n", i);
					memcpy(diagonal_parity, tempBlock[k + 1] + i * symbolSize, symbolSize);
					id = i;
					symbol = 0;
					for(uint32_t j = 0; j < k - 1; ++j){
						bitwiseXor(diagonal_parity, diagonal_parity, tempBlock[id] + symbol * symbolSize, symbolSize);
						id = (id - 1 + k) % k; 
						symbol++;
					} 
					break;
				}
		}

		// Apply Diagonal Parity Adjuster
		for(auto block_symbol : needDiagonalFix){
			char* bufPtr = tempBlock[block_symbol.first] + block_symbol.second * symbolSize;
			debug("Apply Diagonal Parity Adjuster %" PRIu32 ":%" PRIu32 "\n", block_symbol.first, block_symbol.second);
			bitwiseXor(bufPtr, bufPtr, diagonal_parity, symbolSize);
		}
	}

	SegmentData segmentData;
	segmentData.info.segmentId = blockDataList[blockList[0].first].info.segmentId;
	segmentData.info.segmentSize = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	for(uint32_t i = 0; i < k - 1; ++i)
		memcpy(segmentData.buf + i * blockSize, tempBlock[i], blockSize);
	memcpy(segmentData.buf + (k - 1) * blockSize, tempBlock[k - 1], segmentSize - (k - 1) * blockSize);

	for(uint32_t i = 0; i < n; ++i)
		MemoryPool::getInstance().poolFree(tempBlock[i]);
	MemoryPool::getInstance().poolFree(repair_symbol);
	MemoryPool::getInstance().poolFree(diagonal_parity);
	return segmentData;
}

block_list_t EvenOddCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = roundTo(segmentSize, k * (k - 1)) / k;
	//const uint32_t symbolSize = blockSize / (k - 1);
	const uint32_t numOfFailed = (uint32_t)std::count(blockStatus.begin(), blockStatus.end(), false);

	if(numOfFailed > 2) {
		debug_error("Cannot Fix More than 2 Nodes Failure (%" PRIu32 ")\n",numOfFailed);
		exit(-1);
	}

	block_list_t requiredBlockList;

	for(uint32_t i = 0; i < n; ++i) {
		if(blockStatus[i] == true) {
			offset_length_t symbol = make_pair(0, blockSize);
			vector<offset_length_t> symbolList = { symbol };
			symbol_list_t blockSymbols = make_pair(i, symbolList);
			requiredBlockList.push_back(blockSymbols);
		}
		if(requiredBlockList.size() >= k)
			break;
	}

	return requiredBlockList;
}

block_list_t EvenOddCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	//const uint32_t n = getBlockCountFromSetting(setting);
	//const uint32_t k = n - 2;
	//const uint32_t blockSize = roundTo(segmentSize, k * (k - 1)) / k;
	//const uint32_t symbolSize = blockSize / (k - 1);
	const uint32_t numOfFailed = (uint32_t)std::count(blockStatus.begin(), blockStatus.end(), false);

	if(numOfFailed > 2) {
		debug_error("Cannot Fix More than 2 Nodes Failure (%" PRIu32 ")\n",numOfFailed);
		exit(-1);
	} else if(numOfFailed == 2) { // Two Nodes Failure
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
	}

	block_list_t requiredBlockList;

	// One Node Failure
	
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
