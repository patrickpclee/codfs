#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "rdpcoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

RDPCoding::RDPCoding() {

}

RDPCoding::~RDPCoding() {

}

vector<BlockData> RDPCoding::encode(SegmentData segmentData, string setting) {
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentData.info.segmentSize, k);
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
		if (i * blockSize >= segmentData.info.segmentSize) {
			//Zero Padding
		} else if ((i + 1) * blockSize > segmentData.info.segmentSize) {
			memcpy(blockData.buf, bufPos,
					segmentData.info.segmentSize - i * blockSize);
			memset(blockData.buf + segmentData.info.segmentSize - i * blockSize, 0,
					(i + 1) * blockSize - segmentData.info.segmentSize);
		} else
			memcpy(blockData.buf, bufPos, blockSize);

		// Compute Row Parity Block
		if (i == 0) {
			memcpy(blockDataList[k].buf, blockData.buf, blockSize);
		} else {
			bitwiseXor(blockDataList[k].buf, blockDataList[k].buf, blockData.buf, blockSize);
		}

		// Compute Diagonal Parity Block
		for(uint32_t j = 0; j < k; ++j) {		
			if(d_group == k) {
				// Missing Diagonal
			} else
				bitwiseXor(blockDataList[k + 1].buf + d_group * symbolSize, blockDataList[k + 1].buf + d_group * symbolSize, blockData.buf + j * symbolSize, symbolSize);

			d_group = (d_group + 1) % (k + 1);
		}

		blockDataList[i] = blockData;
	}

	// Adjust to Diagonal Parity
	bitwiseXor(blockDataList[k+1].buf, blockDataList[k].buf + symbolSize , blockDataList[k+1].buf, (k - 1) * symbolSize);

	//debug("Block Data List Count: %zu\n", blockDataList.size());
	return blockDataList;
}

char** RDPCoding::repairDataBlocks(vector<BlockData> &blockDataList, block_list_t &blockList, uint32_t segmentSize, string setting) {
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, k);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);

	uint32_t row_group_erasure[k];
	uint32_t diagonal_group_erasure[k + 1];
	bool disk_block_status[n][k];
	std::fill_n (row_group_erasure, k, k + 1);
	std::fill_n (diagonal_group_erasure, k + 1, k + 1);

	uint32_t numOfFailedDisk = n;
	uint32_t disk_block_erasure[n];
	std::fill_n (disk_block_erasure, n, k);

	char** tempBlock;
	tempBlock = (char**)MemoryPool::getInstance().poolMalloc(sizeof(char*) * n);

	for(uint32_t i = 0; i < n; ++i) {
		tempBlock[i] = MemoryPool::getInstance().poolMalloc(blockSize);
		std::fill_n(disk_block_status[i], k, false);
	}

	for(auto blockSymbols: blockList) {
		vector<offset_length_t> symbolList = blockSymbols.second;
		uint32_t id = blockSymbols.first;
		disk_block_erasure[id] = 0;
		--numOfFailedDisk;
		uint32_t offset = 0;
		for(auto symbol : symbolList) {
			memcpy(tempBlock[id] + symbol.first, blockDataList[id].buf + offset, symbol.second);
			offset += symbol.second;
			uint32_t firstSymbolId = symbol.first / symbolSize;
			for(uint32_t i = 0; i < (symbol.second / symbolSize); ++i) {
				uint32_t symbol_id = firstSymbolId + i;
				disk_block_status[id][symbol_id] = true;
				if(id != k + 1) {
					--row_group_erasure[symbol_id];
				}

				uint32_t d_group = (symbol_id + id) % (k + 1);
				--diagonal_group_erasure[d_group];
			}
		}
	}

	uint32_t id;
	uint32_t symbol;
	uint32_t target_id;
	uint32_t target_symbol;
	char* repair_symbol = MemoryPool::getInstance().poolMalloc(symbolSize);
	while(1) {
		/*
		for(uint32_t i = 0; i < k; ++i) {
			debug("Row %" PRIu32 ": %" PRIu32 " Erasures, Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", i, row_group_erasure[i], i, diagonal_group_erasure[i]);
		}		
		debug("                   Diagonal %" PRIu32 ": %" PRIu32 " Erasures\n", k, diagonal_group_erasure[k]);
		*/

		if(numOfFailedDisk == 0)
			break;

		target_id = (uint32_t) - 1;
		target_symbol = (uint32_t) - 1;
		memset(repair_symbol, 0, symbolSize);
		
		for(uint32_t i = 0; i < k; ++i) {
			// Fix by Row
			if(row_group_erasure[i] == 1) {
				symbol = i;
				for(uint32_t j = 0; j < k + 1; ++j) {
					id = j;
					if(disk_block_status[id][symbol] == false) {
						target_id = id;
						target_symbol = symbol;
					} else {
						bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
					}
				}

				uint32_t d_group = (target_symbol + target_id) % (k + 1);
				--diagonal_group_erasure[d_group];

				row_group_erasure[symbol] = 0;
				//debug("Fix %" PRIu32 ":%" PRIu32 " with row %" PRIu32 "\n", target_id, target_symbol, symbol);
				break;
			}
			
			// Fix by Diagonal
			if(diagonal_group_erasure[i] == 1) {
				id = i;
				symbol = 0;
				for(uint32_t j = 0; j < k + 1; ++j) {
					if(disk_block_status[id][symbol] == false) {
						target_id = id;
						target_symbol = symbol;
					} else {
						bitwiseXor(repair_symbol, repair_symbol, tempBlock[id] + symbol * symbolSize, symbolSize);
					}
					if (id == 0) id = k + 1;
					else {
						--id;
						++symbol;
					}
				}
				diagonal_group_erasure[i] = 0;
				if(target_id != k + 1)
					--row_group_erasure[target_symbol];

				//debug("Fix %" PRIu32 ":%" PRIu32 " with diagonal %" PRIu32 "\n", target_id, target_symbol, i);
				break;
			}
		}

		if(target_id == (uint32_t)-1) {
			//debug_error("%s\n", "Cannot fix any more");
			break;
		}

		disk_block_status[target_id][target_symbol] = true;
		memcpy(tempBlock[target_id] + target_symbol * symbolSize, repair_symbol, symbolSize);
		--disk_block_erasure[target_id];
		//debug("Disk %" PRIu32 " Erasure %" PRIu32 "\n", target_id, disk_block_erasure[target_id]);
		if(disk_block_erasure[target_id] == 0)
			--numOfFailedDisk;
	}
	MemoryPool::getInstance().poolFree(repair_symbol);
	return tempBlock;
}

block_list_t RDPCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, k);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);
	const uint32_t numOfFailed = (uint32_t)std::count(blockStatus.begin(), blockStatus.end(), false);

	if(numOfFailed > 2) {
		debug_error("Cannot Fix More than 2 Nodes Failure (%" PRIu32 ")\n",numOfFailed);
		exit(-1);
	} else if(numOfFailed == 2) { // Two Nodes Failure
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
	}

	// Diagonal Failed
	if (!blockStatus[k+1])
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);

	// Optimisation From 
	// ``A Hybrid Approach of Failed Disk Recovery Using RAID-6 Codes: Algorithms and Performance Evaluation''
	// L.P. Xiang, Y.L. Xu, John C.S. Lui, Q. Chang, Y.B. Pan and R.H. Li.
	// ACM Transactions on Storage, 7(3), October, 2011.

	uint32_t failedDisk = failedBlocks[0];

	bool requiredSymbol[n][k];
	for(uint32_t i = 0; i < n; ++i){
		std::fill_n(requiredSymbol[i], k, false);
	}

	uint32_t numOfRows = k / 2;
	uint32_t numOfDiagonals = k - numOfRows;
	bool fixedSymbol[k];
	std::fill_n(fixedSymbol, k, false);

	uint32_t symbol;


	// Force Row Fix for Symbol on Diagonal K
	symbol = k - failedDisk;
	if(symbol < k) {
		for(uint32_t j = 0; j < k + 1; ++j) {
			if(j != failedDisk){ 
				requiredSymbol[j][symbol] = true;
			}
		}
		fixedSymbol[symbol] = true;
		//debug("Row %" PRIu32 "\n", symbol);
		--numOfRows;
	}
	

	symbol = 0;
	for(uint32_t i = 0; i < numOfRows; ++i) {
		while(fixedSymbol[symbol])
			++symbol;
		for(uint32_t j = 0; j < k + 1; ++j) {
			if(j != failedDisk){ 
				requiredSymbol[j][symbol] = true;
			}
		}
		fixedSymbol[symbol] = true;
		//debug("Row %" PRIu32 "\n", symbol);
		++symbol;
	}
	
	for(uint32_t i = 0; i < numOfDiagonals; ++i) {
		while(fixedSymbol[symbol])
			++symbol;
		uint32_t d_group = (symbol + failedDisk) % (k + 1);
		uint32_t temp_id = d_group;
		uint32_t temp_symbol = 0;
		for(uint32_t j = 0; j < k + 1; ++j) {
			if(temp_id != failedDisk){
				requiredSymbol[temp_id][temp_symbol] = true;
			}
			if(temp_id == 0)
				temp_id = k + 1;
			else {
				--temp_id;
				++temp_symbol;
			}
		}
		fixedSymbol[symbol] = true;
		//debug("Diagonal %" PRIu32 "\n", d_group);
		++symbol;
	}

	block_list_t requiredBlockList;
	// Convert BitMap to Block Symbol List
	for(uint32_t i = 0; i < n; ++i){
		vector<offset_length_t> symbolList;
		uint32_t startSymbol = 0;
		uint32_t length = 0;
		for(uint32_t j = 0; j < k; ++j){
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

vector<BlockData> RDPCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, k);
	//const uint32_t symbolSize = getSymbolSize(blockSize, k);

	uint32_t segmentId = blockData[symbolList[0].first].info.segmentId;

	char** tempBlock = repairDataBlocks(blockData, symbolList, segmentSize, setting);

	vector<BlockData> blockDataList;
	for(auto targetBlock : repairBlockIdList) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = targetBlock;
		blockData.info.blockSize = blockSize;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);

		memcpy(blockData.buf, tempBlock[targetBlock], blockSize);
		blockDataList.push_back(blockData);
	}

	for(uint32_t i = 0; i < n; ++i)
		MemoryPool::getInstance().poolFree(tempBlock[i]);
	MemoryPool::getInstance().poolFree((char*)tempBlock);
	
	return blockDataList;
}
