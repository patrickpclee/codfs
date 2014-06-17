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
	const uint32_t blockSize = getBlockSize(segmentData.info.segLength, setting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);

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
		if (i * blockSize >= segmentData.info.segLength) {
			//Zero Padding
		} else if ((i + 1) * blockSize > segmentData.info.segLength) {
			memcpy(blockData.buf, bufPos,
					segmentData.info.segLength - i * blockSize);
			memset(blockData.buf + segmentData.info.segLength - i * blockSize, 0,
					(i + 1) * blockSize - segmentData.info.segLength);
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

	//debug("Block Data List Count: %zu\n", blockDataList.size());
	return blockDataList;
}

SegmentData EvenOddCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &blockList, uint32_t segmentSize, string setting) {
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = this->getBlockSize(segmentSize, setting);
	//const uint32_t symbolSize = this->getSymbolSize(blockSize, k);

	char** tempBlock = this->repairDataBlocks(blockDataList, blockList, segmentSize, setting);
	SegmentData segmentData;
	segmentData.info.segmentId = blockDataList[blockList[0].first].info.segmentId;
	segmentData.info.segLength = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	for(uint32_t i = 0; i < k - 1; ++i)
		memcpy(segmentData.buf + i * blockSize, tempBlock[i], blockSize);
	memcpy(segmentData.buf + (k - 1) * blockSize, tempBlock[k - 1], segmentSize - (k - 1) * blockSize);

	for(uint32_t i = 0; i < n; ++i)
		MemoryPool::getInstance().poolFree(tempBlock[i]);
	MemoryPool::getInstance().poolFree((char*)tempBlock);
	return segmentData;
}


char** EvenOddCoding::repairDataBlocks(vector<BlockData> &blockDataList, block_list_t &blockList, uint32_t segmentSize, string setting, bool recovery) {
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);
	uint32_t row_group_erasure[k - 1];
	uint32_t diagonal_group_erasure[k];
	uint32_t datadisk_block_erasure[k];
	bool disk_block_status[n][k - 1];
	bool need_diagonal_adjust[k][k - 1];
	uint32_t numOfFailedDataDisk = k;
	std::fill_n (row_group_erasure, k - 1, k + 1);
	std::fill_n (diagonal_group_erasure, k - 1, k);
	std::fill_n (datadisk_block_erasure, k, k - 1);
	char* diagonal_adjuster = MemoryPool::getInstance().poolMalloc(symbolSize);
	char* repair_symbol = MemoryPool::getInstance().poolMalloc(symbolSize);

	bool use_diagonal = false;
	bool use_row = false;

	char** tempBlock;
	tempBlock = (char**)MemoryPool::getInstance().poolMalloc(sizeof(char*) * n);
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
						diagonal_group = firstSymbolId + i;
					}
					--diagonal_group_erasure[diagonal_group];
					//debug("%" PRIu32 ":%" PRIu32 " Diagonal Group %" PRIu32 "\n", id, firstSymbolId + i, diagonal_group);
					if(diagonal_group == k - 1) {
						//debug("bitwise %" PRIu32 ":%" PRIu32 "to adjuster\n", id, firstSymbolId + i);
						bitwiseXor(diagonal_adjuster, tempBlock[id] + (firstSymbolId + i) * symbolSize, diagonal_adjuster, symbolSize);	
					}
				}
			}
		}
	}


	bool diagonal_adjuster_failed = !(diagonal_group_erasure[k - 1] == 0);
	bool diagonal_clean[k];
	std::fill_n(diagonal_clean, k , true);
	vector<block_symbol_t> needDiagonalFix = {};

	// Fix Failed Data Disk
	uint32_t id;
	uint32_t symbol;
	uint32_t target_id;
	uint32_t target_symbol;
	while(1) {
		/*
		for(uint32_t i = 0; i < k - 1; ++i){
			debug("Row %" PRIu32 ": %" PRIu32 " Erasures, Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", i, row_group_erasure[i], i, diagonal_group_erasure[i]);
		}
		debug("Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", k - 1, diagonal_group_erasure[k - 1]);
		*/

		if(numOfFailedDataDisk == 0)
			break;

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
							//debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
							cur_symbol_need_adjust = !cur_symbol_need_adjust;
						}
					}
				}

				if(use_diagonal) {
					uint32_t diagonal_group = (target_symbol + target_id) % k;
					// Fix Diagonal Parity Adjuster
					if(diagonal_group == k - 1) {
						//debug("bitwise %" PRIu32 ":%" PRIu32 "to adjuster\n", target_id, target_symbol);
						bitwiseXor(diagonal_adjuster, repair_symbol, diagonal_adjuster, symbolSize);
					}
					--diagonal_group_erasure[diagonal_group];
					diagonal_clean[diagonal_group] = diagonal_clean[diagonal_group] != cur_symbol_need_adjust;
				}

				--row_group_erasure[symbol];

				//debug("Fix %" PRIu32 ":%" PRIu32 " with row %" PRIu32 "\n", target_id, target_symbol, symbol);
				break;
			}

			// Fix by Diagonal
			if(use_diagonal && (diagonal_group_erasure[i] == 1)) {
				cur_symbol_need_adjust = true;
				id = i;
				symbol = 0;
				for(uint32_t j = 0; j < k - 1; ++j){
					//debug("%" PRIu32 ":%" PRIu32 "\n",id,symbol);
					if(disk_block_status[id][symbol] == false) {
						target_id = id;
						target_symbol = symbol;
					} else {
						bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
						if(need_diagonal_adjust[id][symbol]){
							//debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
							cur_symbol_need_adjust = !cur_symbol_need_adjust;
						}
					}
					id = (id - 1 + k) % k;
					++symbol;
				}
				bitwiseXor(repair_symbol, repair_symbol, tempBlock[k + 1] + i * symbolSize, symbolSize);

				diagonal_clean[i] = diagonal_clean[i] != cur_symbol_need_adjust;

				diagonal_group_erasure[i] = 0;
				if(use_row)
					--row_group_erasure[target_symbol];
				//debug("Fix %" PRIu32 ":%" PRIu32 " with diagonal %" PRIu32 "\n", target_id, target_symbol, i);
				break;
			}
		}

		// Fix Diagonal Adjuster
		if(use_diagonal && (target_id == (uint32_t)-1) && (diagonal_group_erasure[k - 1] == 1)) {
			//debug("%s\n","Fix Diagonal Adjuster");
			cur_symbol_need_adjust = true;
			id = k - 1;
			symbol = 0;
			for(uint32_t i = 0; i < k - 1; ++i) {
				if(disk_block_status[id][symbol]){
					bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
					if(need_diagonal_adjust[id][symbol]){
						//debug("%" PRIu32 ":%" PRIu32 " toggles need adjustment\n", id, symbol);
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
			debug_error("%s\n","Cannot fix any more");
			break;
		}

		if(cur_symbol_need_adjust) {
			//debug("%" PRIu32 ":%" PRIu32 " need diagonal adjustment\n", target_id, target_symbol);
			need_diagonal_adjust[target_id][target_symbol] = true;
			needDiagonalFix.push_back(make_pair(target_id, target_symbol));
		}

		disk_block_status[target_id][target_symbol] = true;

		memcpy(tempBlock[target_id] + target_symbol * symbolSize, repair_symbol, symbolSize);

		--datadisk_block_erasure[target_id];
		//debug("Disk %" PRIu32 " Erasure %" PRIu32 "\n", target_id, datadisk_block_erasure[target_id]);
		if(datadisk_block_erasure[target_id] == 0){
			--numOfFailedDataDisk;
			//debug("Disk %" PRIu32 " Fixed, %" PRIu32 " Failure Left\n", target_id, numOfFailedDataDisk);
		}
	}

	if(use_diagonal) {
		// Find a Clean Diagonal Parity
		//if(diagonal_adjuster_failed && !diagonal_clean[k - 1]) {
		if(diagonal_adjuster_failed){
			for(uint32_t i = 0; i < k - 1; ++i)
				if(diagonal_clean[i] && (diagonal_group_erasure[i] == 0)){
					//debug("Clean Diagonal Found at %" PRIu32 "\n", i);
					memcpy(diagonal_adjuster, tempBlock[k + 1] + i * symbolSize, symbolSize);
					id = i;
					symbol = 0;
					for(uint32_t j = 0; j < k - 1; ++j){
						bitwiseXor(diagonal_adjuster, diagonal_adjuster, tempBlock[id] + symbol * symbolSize, symbolSize);
						id = (id - 1 + k) % k; 
						symbol++;
					} 
					break;
				}
		}

		// Apply Diagonal Parity Adjuster
		for(auto block_symbol : needDiagonalFix){
			char* bufPtr = tempBlock[block_symbol.first] + block_symbol.second * symbolSize;
			//debug("Apply Diagonal Parity Adjuster %" PRIu32 ":%" PRIu32 "\n", block_symbol.first, block_symbol.second);
			bitwiseXor(bufPtr, bufPtr, diagonal_adjuster, symbolSize);
		}
	}

	MemoryPool::getInstance().poolFree(repair_symbol);
	MemoryPool::getInstance().poolFree(diagonal_adjuster);
	return tempBlock;
}

block_list_t EvenOddCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = this->getBlockSize(segmentSize, setting);
	//debug("%" PRIu32 "\n", blockSize);
	//const uint32_t symbolSize = this->getSymbolSize(blockSize, k);
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

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);
	const uint32_t numOfFailed = (uint32_t)std::count(blockStatus.begin(), blockStatus.end(), false);

	if(numOfFailed > 2) {
		debug_error("Cannot Fix More than 2 Nodes Failure (%" PRIu32 ")\n",numOfFailed);
		exit(-1);
	} else if(numOfFailed == 2) { // Two Nodes Failure
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
	}

	if (!blockStatus[k] || !blockStatus[k+1])
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);

	// Optimisation From 
	// ``A Hybrid Approach of Failed Disk Recovery Using RAID-6 Codes: Algorithms and Performance Evaluation''
	// L.P. Xiang, Y.L. Xu, John C.S. Lui, Q. Chang, Y.B. Pan and R.H. Li.
	// ACM Transactions on Storage, 7(3), October, 2011.

	uint32_t failedDisk = failedBlocks[0];

	bool requiredSymbol[n][k - 1];
	for(uint32_t i = 0; i < n; ++i){
		std::fill_n(requiredSymbol[i], k - 1, false);
	}

	uint32_t symbol = 0;
	uint32_t id = k - 1;
	uint32_t numOfRows = (k - 1) / 2;
	uint32_t numOfDiagonals = k - 1 - numOfRows;
	bool fixedSymbol[k - 1];
	std::fill_n(fixedSymbol, k - 1, false);

	// Read Diagonal Adjuster
	for(uint32_t i = 0 ; i < k - 1; ++i) {
		if(id == failedDisk) {
			for(uint32_t j = 0; j < k + 1; ++j) {
				if(j == failedDisk) continue;
				requiredSymbol[j][symbol] = true;
			}
			fixedSymbol[symbol] = true;
			--numOfRows;
		} else
			requiredSymbol[id][symbol] = true;
		--id;
		++symbol;
	}

	id = failedDisk;
	symbol = 0;
	// Read k - 1 / 2 rows
	for(uint32_t i = 0; i < numOfRows; ++i) {
		while(fixedSymbol[symbol])
			++symbol;
		for(uint32_t j = 0; j < k + 1; ++j) {
			if(j == failedDisk) continue;
			requiredSymbol[j][symbol] = true;
		}
		fixedSymbol[symbol] = true;
		//debug("Row %" PRIu32 "\n", symbol);
		++symbol;
	}

	uint32_t diagonal_group;
	// Read rest diagonals
	for(uint32_t i = 0; i < numOfDiagonals; ++i) {
		while(fixedSymbol[symbol])
			++symbol;
		diagonal_group = (symbol + failedDisk) % k;
		uint32_t temp_id = diagonal_group;
		uint32_t temp_symbol = 0;
		for(uint32_t j = 0; j < k - 1; ++j) {
			if(temp_id != failedDisk)
				requiredSymbol[temp_id][temp_symbol] = true;
			temp_id = (temp_id - 1 + k) % k;
			++temp_symbol;
		}
		requiredSymbol[k + 1][diagonal_group] = true;
		//debug("Diagonal %" PRIu32 "\n", diagonal_group);
		fixedSymbol[symbol] = true;
		++symbol;
	} 

	block_list_t requiredBlockList;
	// Convert BitMap to Block Symbol List
	for(uint32_t i = 0; i < n; ++i){
		vector<offset_length_t> symbolList;
		uint32_t startSymbol = 0;
		uint32_t length = 0;
		for(uint32_t j = 0; j < k - 1; ++j){
			if(requiredSymbol[i][j]) {
				//debug("Require Symbol %" PRIu32 ":%" PRIu32 "\n", i, j);
				if(length == 0)
					startSymbol = j;
				length += symbolSize;
			} else if(length > 0) {
				//debug("%" PRIu32 "-%" PRIu32 ":%" PRIu32 "\n", i,startSymbol, length);
				symbolList.push_back(make_pair(startSymbol * symbolSize, length));
				startSymbol = 0;
				length = 0;
			}
		}
		if(length > 0) {
			//debug("%" PRIu32 "-%" PRIu32 ":%" PRIu32 "\n", i,startSymbol, length);
			symbolList.push_back(make_pair(startSymbol * symbolSize, length));
			startSymbol = 0;
			length = 0;
		}
		if(symbolList.size() > 0) {
			requiredBlockList.push_back(make_pair(i, symbolList));
		}
	}

	return requiredBlockList;
}

vector<BlockData> EvenOddCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockDataList, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);

	uint32_t segmentId = blockDataList[symbolList[0].first].info.segmentId;

	char** tempBlock = repairDataBlocks(blockDataList, symbolList, segmentSize, setting);

	vector<BlockData> repairedBlockDataList;
	bool blockNeeded[n];
	std::fill_n(blockNeeded, n , false);
	for(auto targetBlock : repairBlockIdList) {
		blockNeeded[targetBlock] = true;	
		struct BlockData blockData;
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = targetBlock;
		blockData.info.blockSize = blockSize;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);

		// Row Parity Block
		if(targetBlock == k) {
			for(uint32_t i = 0; i < k; ++i)
				bitwiseXor(tempBlock[k], tempBlock[k], tempBlock[i], blockSize);
		// Diagonal Parity Block
		} else if(targetBlock == k + 1) {
			// Reconstruct Diagonal Adjuster
			char* diagonal_adjuster = MemoryPool::getInstance().poolMalloc(symbolSize);
			uint32_t id = k - 1;
			uint32_t symbol = 0;
			memcpy(diagonal_adjuster, tempBlock[id] + symbol * symbolSize, symbolSize);
			--id;
			++symbol;
			for(uint32_t i = 1; i < k - 1; ++i) {
				bitwiseXor(diagonal_adjuster, diagonal_adjuster, tempBlock[id] + symbol * symbolSize, symbolSize);
				--id;
				++symbol;
			} 

			for(uint32_t i = 0; i < k - 1; ++i) {
				id = i;
				symbol = 0;
				char* targetSymbol = tempBlock[targetBlock] + i * symbolSize;
				for(uint32_t j = 0; j < k - 1; ++j) {
					bitwiseXor(targetSymbol, targetSymbol, tempBlock[id] + symbol * symbolSize, symbolSize);
					id = (id - 1 + k) % k;
					++symbol;
				}
				bitwiseXor(targetSymbol, targetSymbol, diagonal_adjuster, symbolSize);
			}
			MemoryPool::getInstance().poolFree(diagonal_adjuster);
		}
		memcpy(blockData.buf, tempBlock[targetBlock], blockSize);
		blockDataList[targetBlock] = blockData;
		repairedBlockDataList.push_back(blockData);
	}

	for(uint32_t i = 0; i < n; ++i)
		MemoryPool::getInstance().poolFree(tempBlock[i]);
	MemoryPool::getInstance().poolFree((char*)tempBlock);
	
	return repairedBlockDataList;
}

uint32_t EvenOddCoding::getBlockCountFromSetting(string setting) {
	return getParameters(setting);
}

uint32_t EvenOddCoding::getParityCountFromSetting(string setting) {
    // TODO: is this fixed?
	return 2;
}

uint32_t EvenOddCoding::getBlockSize(uint32_t segmentSize, string setting) {
	uint32_t k = getParameters(setting) - 2;
	return roundTo(segmentSize, k * (k - 1)) / k;
}

//
// PRIVATE FUNCTION
//

uint32_t EvenOddCoding::getParameters(string setting) {
	uint32_t n;
	istringstream(setting) >> n;
	return n;
}
