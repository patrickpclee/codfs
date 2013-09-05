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
/*
 * (blockId, symbolIdx) --> parity idx
 * (0, 0) => 0
 * (0, 1) => 0, 1
 * (0, 2) => 1, 2
 * (0, 3) => 2, 3
 * (1, 0) => 1
 * (1, 1) => 0, 2
 * (1, 2) => 1, 3
 * (1, 3) => 2
 * (2, 0) => 2
 * (2, 1) => 0, 3
 * (2, 2) => 1
 * (2, 3) => 0, 2
 * (3, 0) => 3
 * (3, 1) => 0
 * (3, 2) => 0, 1
 * (3, 3) => 1, 2
 */
RDPCoding::RDPCoding() {
    deltaMap[0][0].push_back(0);
    deltaMap[0][1].push_back(0);
    deltaMap[0][1].push_back(1);
    deltaMap[0][2].push_back(1);
    deltaMap[0][2].push_back(2);
    deltaMap[0][3].push_back(2);
    deltaMap[0][3].push_back(3);

    deltaMap[1][0].push_back(1);
    deltaMap[1][1].push_back(0);
    deltaMap[1][1].push_back(2);
    deltaMap[1][2].push_back(1);
    deltaMap[1][2].push_back(3);
    deltaMap[1][3].push_back(2);

    deltaMap[2][0].push_back(2);
    deltaMap[2][1].push_back(0);
    deltaMap[2][1].push_back(3);
    deltaMap[2][2].push_back(1);
    deltaMap[2][3].push_back(0);
    deltaMap[2][3].push_back(2);

    deltaMap[3][0].push_back(3);
    deltaMap[3][1].push_back(0);
    deltaMap[3][2].push_back(0);
    deltaMap[3][2].push_back(1);
    deltaMap[3][3].push_back(1);
    deltaMap[3][3].push_back(2);
}

RDPCoding::~RDPCoding() {

}

vector<BlockData> RDPCoding::encode(SegmentData segmentData, string setting) {
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

char** RDPCoding::repairDataBlocks(vector<BlockData> &blockDataList, block_list_t &blockList, uint32_t segmentSize, string setting, bool recovery) {
	const uint32_t n = getBlockCountFromSetting(setting);
	const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);

	uint32_t row_group_erasure[k];
	uint32_t diagonal_group_erasure[k + 1];
	bool disk_block_status[n][k];
	std::fill_n (row_group_erasure, k, k + 1);
	std::fill_n (diagonal_group_erasure, k + 1, k + 1);

	uint32_t numOfFailedDisk = n;
	uint32_t numOfFailedDataDisk = k;
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
		if(id < k)
			--numOfFailedDataDisk;
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

	if(!recovery && (numOfFailedDataDisk == 0))
		return tempBlock;

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
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
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
		vector<BlockData> &blockDataList, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	const uint32_t n = getBlockCountFromSetting(setting);
	//const uint32_t k = n - 2;
	const uint32_t blockSize = getBlockSize(segmentSize, setting);
	//const uint32_t symbolSize = getSymbolSize(blockSize, k);

	uint32_t segmentId = blockDataList[symbolList[0].first].info.segmentId;

	char** tempBlock = repairDataBlocks(blockDataList, symbolList, segmentSize, setting, true);

	vector<BlockData> repairedBlockDataList;
	for(auto targetBlock : repairBlockIdList) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = targetBlock;
		blockData.info.blockSize = blockSize;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);

		memcpy(blockData.buf, tempBlock[targetBlock], blockSize);
		blockDataList[targetBlock] = blockData;
		repairedBlockDataList.push_back(blockData);
	}

	for(uint32_t i = 0; i < n; ++i)
		MemoryPool::getInstance().poolFree(tempBlock[i]);
	MemoryPool::getInstance().poolFree((char*)tempBlock);
	
	return repairedBlockDataList;
}

uint32_t RDPCoding::getBlockSize(uint32_t segmentSize, string setting) {
	uint32_t k = getParameters(setting) - 2;
	return roundTo(segmentSize, k * k) / k;
}

uint32_t RDPCoding::getParityCountFromSetting (string setting) {
    // TODO: is this fixed?
    return 2;
}

/* Compute Delta for RDPCoding
 * First check which blockId it updates
 * symbolNum * symbolSize = blockSize
 * Xor the old and new delta first
 * Repack the offlenVector to align symbolsize, vector<offset_length_t> offlens[symbolNum]
 * (blockId, symbolIdx) --> parity idx
 * (0, 0) => 0
 * (0, 1) => 0, 1
 * (0, 2) => 1, 2
 * (0, 3) => 2, 3
 * (1, 0) => 1
 * (1, 1) => 0, 2
 * (1, 2) => 1, 3
 * (1, 3) => 2
 * (2, 0) => 2
 * (2, 1) => 0, 3
 * (2, 2) => 1
 * (2, 3) => 0, 2
 * (3, 0) => 3
 * (3, 1) => 0
 * (3, 2) => 0, 1
 * (3, 3) => 1, 2
 */ 
vector<BlockData> RDPCoding::computeDelta(BlockData oldBlock, BlockData newBlock,
        vector<offset_length_t> offsetLength, vector<uint32_t> parityBlockIdVector) {

    debug("XXXXX NEW BLOCK SID = %" PRIu64 "\n", newBlock.info.segmentId);
    debug("XXXXX OLD BLOCK SID = %" PRIu64 "\n", oldBlock.info.segmentId);

    const string codingSetting = newBlock.info.codingSetting;
	const uint32_t k = getBlockCountFromSetting(codingSetting) - 2;
    const uint64_t segmentSize = newBlock.info.segmentSize;
	const uint32_t blockSize = getBlockSize(segmentSize, codingSetting);
	const uint32_t symbolSize = getSymbolSize(blockSize, k);

    vector<BlockData> deltas (parityBlockIdVector.size());

    // Compute first parity, just xor
    uint32_t combinedLength = getCombinedLength(offsetLength);
    BlockData &delta = deltas[0];
    delta = oldBlock;
    delta.info.blockId = parityBlockIdVector[0];
    delta.buf = MemoryPool::getInstance().poolMalloc(combinedLength);
    bitwiseXor(delta.buf, oldBlock.buf, newBlock.buf, combinedLength);
    
    // Computer second parity
    uint32_t blockId = newBlock.info.blockId;
    char *pbuf = MemoryPool::getInstance().poolMalloc(blockSize);
    memset(pbuf, 0 , blockSize);
    vector<offset_length_t> d2offlen;

    uint32_t comboffset = 0;
    for (auto& offlen : offsetLength) {
        uint32_t offset = offlen.first, remain = offlen.second;
        while (remain) {
            uint32_t symbolIdx = offset / symbolSize;
            uint32_t updateLen = min (remain, symbolSize * (symbolIdx + 1) - offset);
            for (uint32_t paritySymbolIdx : deltaMap[blockId][symbolIdx]) {
                uint32_t pbufoffset = offset+symbolSize*(paritySymbolIdx-symbolIdx);
                bitwiseXor(pbuf+pbufoffset, pbuf+pbufoffset, delta.buf+comboffset, updateLen);
                d2offlen.push_back(make_pair(pbufoffset, updateLen));
            }
            comboffset += updateLen;
            offset += updateLen;
            remain -= updateLen;
        }
    }

    // pack pbuf d2offlen
    vector<offset_length_t> packedVector;
    packedVector.clear();
    sort(d2offlen.begin(), d2offlen.end());
    uint32_t curOffsetStart = d2offlen[0].first;
    uint32_t curOffsetEnd = d2offlen[0].first + d2offlen[0].second;
    for (uint32_t i = 1; i < d2offlen.size(); i++) {
        if (d2offlen[i].first > curOffsetEnd) {
            // Seal current offset,length
            packedVector.push_back(make_pair(curOffsetStart, curOffsetEnd-curOffsetStart));
            curOffsetStart = d2offlen[i].first;
            curOffsetEnd = d2offlen[i].first + d2offlen[i].second;
        } else {
            curOffsetEnd = max(curOffsetEnd, d2offlen[i].first + d2offlen[i].second);
        }
    }
    // Seal last offset,length
    packedVector.push_back(make_pair(curOffsetStart, curOffsetEnd-curOffsetStart));

    // create second delta
    BlockData& delta1 = deltas[1];
    delta1.info = oldBlock.info;
    delta1.info.blockId = parityBlockIdVector[1];
    delta1.info.offlenVector = packedVector;
    delta1.info.blockSize = getCombinedLength(packedVector);
    delta1.buf = MemoryPool::getInstance().poolMalloc(delta1.info.blockSize);
    

    comboffset = 0;
    for (auto& offlen : delta1.info.offlenVector) {
        debug_cyan ("second delta1 offset = %" PRIu32 " and length = %" PRIu32 "\n", offlen.first, offlen.second);
        memcpy(delta1.buf + comboffset, pbuf + offlen.first, offlen.second);
        comboffset += offlen.second;
    }

    MemoryPool::getInstance().poolFree(pbuf);
    return deltas;
}
