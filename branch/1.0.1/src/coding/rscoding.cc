#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "rscoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

RSCoding::RSCoding() {

}

RSCoding::~RSCoding() {

}

vector<BlockData> RSCoding::encode(SegmentData segmentData, string setting) {

	vector<struct BlockData> blockDataList;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(
			(roundTo(segmentData.info.segLength, k) / k), 4);

	if (k <= 0 || m < 0 || (w != 8 && w != 16 && w != 32)
			|| (w <= 16 && k + m > ((uint32_t)1 << w))) {
		cerr << "Bad Parameters" << endl;
		exit(-1);
	}

	int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

	char **data, **code;
	data = talloc<char*, uint32_t>(k);

	for (uint32_t i = 0; i < k; i++) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = size;

		blockData.buf = MemoryPool::getInstance().poolMalloc(size);
		char* bufPos = segmentData.buf + i * size;
		
		if (i * size >= segmentData.info.segLength) {
			//Zero Padding
		} else if ((i + 1) * size > segmentData.info.segLength) {
			memcpy(blockData.buf, bufPos,
					segmentData.info.segLength - i * size);
			memset(blockData.buf + segmentData.info.segLength - i * size, 0,
					(i + 1) * size - segmentData.info.segLength);
		} else
			memcpy(blockData.buf, bufPos, size);


		/*
		if (i == k - 1) {
			//memset(blockData.buf, 0, size);
			memcpy(blockData.buf, bufPos,
					segmentData.info.segmentSize - i * size);
			memset(blockData.buf + segmentData.info.segmentSize - i * size, 0,
					k * size - segmentData.info.segmentSize);
		} else
			memcpy(blockData.buf, bufPos, size);
		*/

		data[i] = blockData.buf;
		//data[i] = talloc<char, uint32_t>(size);
		//memcpy(data[i], blockData.buf, size);

		blockDataList.push_back(blockData);
	}

	if (m == 0){
		tfree(data);
		free(matrix);
		return blockDataList;
	}

	code = talloc<char*, uint32_t>(m);
	for (uint32_t i = 0; i < m; i++) {
		code[i] = talloc<char, uint32_t>(size);
	}

	jerasure_matrix_encode(k, m, w, matrix, data, code, size);

	for (uint32_t i = 0; i < m; i++) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = k + i;
		blockData.info.blockSize = size;

		blockData.buf = code[i];
		//blockData.buf = MemoryPool::getInstance().poolMalloc(size);
		//memcpy(blockData.buf, code[i], size);

		blockDataList.push_back(blockData);
	}

	// free memory
	tfree(data);
	tfree(code);
	free(matrix);

	return blockDataList;
}

SegmentData RSCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {


	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(roundTo(segmentSize, k) / k, 4);
	uint32_t numOfFailedDataDisk = k;

	// transform symbolList to blockIdList
	vector<uint32_t> blockIdList;
	for (auto blockSymbol : symbolList) {
		blockIdList.push_back(blockSymbol.first);
		if(blockSymbol.first < k) {
			--numOfFailedDataDisk;
		}
	}

	if (blockIdList.size() < k) {
		cerr << "Not enough blocks for decode " << blockIdList.size() << endl;
		exit(-1);
	}

	struct SegmentData segmentData;

	// copy segmentID from first available block
	segmentData.info.segmentId = blockDataList[blockIdList[0]].info.segmentId;
	segmentData.info.segLength = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	set<uint32_t> blockIdListSet(blockIdList.begin(), blockIdList.end());

	//Optimization for no Data Disk Erasure
	if(numOfFailedDataDisk == 0) {
		uint64_t offset = 0;
		uint32_t i = 0;
		uint32_t copySize = size;
		while(offset < segmentSize) {
			if((offset + size) > segmentSize) {
				copySize = segmentSize - offset;
			}
			memcpy(segmentData.buf + offset, blockDataList[i].buf, copySize);
			++i;
			offset += copySize;
		}
		/*
		for (uint32_t i = 0; i < k - 1; i++) {
			memcpy(segmentData.buf + offset, blockDataList[i].buf, size);
			offset += size;
		}
		memcpy(segmentData.buf + offset, blockDataList[k - 1].buf, segmentSize - offset);
		*/
		return segmentData;
	}

	int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
	char **data, **code;
	int *erasures;
	int j = 0;

	data = talloc<char*, uint32_t>(k);
	code = talloc<char*, uint32_t>(m);
	erasures = talloc<int, uint32_t>(k + m - blockIdList.size() + 1);

	for (uint32_t i = 0; i < k + m; i++) {
		i < k ? data[i] = talloc<char, uint32_t>(size) : code[i - k] =
			talloc<char, uint32_t>(size);
		if (blockIdListSet.count(i) > 0) {
			i < k ? memcpy(data[i], blockDataList[i].buf, size) : 
				memcpy(code[i - k], blockDataList[i].buf, size);
		} else {
			erasures[j++] = i;
		}
	}
	erasures[j] = -1;

	jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

	/*
	for (uint32_t i = 0; i < k + m - blockIdList.size(); i++) {
		struct BlockData temp;
		temp.info.segmentId = segmentData.info.segmentId;
		temp.info.blockId = erasures[i];
		temp.info.blockSize = size;

		temp.buf = MemoryPool::getInstance().poolMalloc(size);
		memcpy(temp.buf,
				(uint32_t) erasures[i] < k ?
				data[erasures[i]] : code[erasures[i] - k], size);

		blockDataList[erasures[i]] = temp;
	}
	*/

	uint64_t offset = 0;
	for (uint32_t i = 0; i < k - 1; i++) {
		memcpy(segmentData.buf + offset, data[i], size);
		offset += size;
	}
	memcpy(segmentData.buf + offset, data[k - 1], segmentSize - offset);


	// free memory
	for (uint32_t i = 0; i < k; i++) {
		tfree(data[i]);
	}
	tfree(data);

	for (uint32_t i = 0; i < m; i++) {
		tfree(code[i]);
	}
	tfree(code);
	tfree(erasures);
	tfree(matrix);

	return segmentData;
}

block_list_t RSCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(blockStatus.begin(), blockStatus.end(),
			false);

	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	//const uint32_t noOfDataStripes = k + m;
	const uint32_t noOfDataStripes = k;
	vector<uint32_t> requiredBlocks;
	requiredBlocks.reserve(noOfDataStripes);

	if ((uint32_t) failedOsdCount > m) {
		return {};
	}

	for (uint32_t i = 0; i < k+m; i++) {
		if (blockStatus[i] != false) {
			requiredBlocks.push_back(i);
		}
		if (requiredBlocks.size() >= k) {
			break;
		}
	}

	const uint32_t blockSize = roundTo(roundTo(segmentSize, noOfDataStripes) / noOfDataStripes, 4);
	//uint32_t blockSize = roundTo(segmentSize, noOfDataStripes)
	//		/ noOfDataStripes;
	block_list_t requiredBlockSymbols;
	for (uint32_t i : requiredBlocks) {
		offset_length_t symbol = make_pair(0, blockSize);
		vector<offset_length_t> symbolList = { symbol };
		symbol_list_t blockSymbols = make_pair(i, symbolList);
		requiredBlockSymbols.push_back(blockSymbols);
	}

	return requiredBlockSymbols;
}

block_list_t RSCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	// for raid-6, same as download
	return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
}

vector<BlockData> RSCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	string blockIdString;
	for (auto block : repairBlockIdList) {
		blockIdString += to_string(block) + " ";
	}

	debug_yellow("Start repairBlocks for %s\n", blockIdString.c_str());

	vector<BlockData> ret;
	//decode(blockData, symbolList, segmentSize, setting);
	vector<uint32_t> blockIdList;
	for (auto blockSymbol : symbolList) {
		blockIdList.push_back(blockSymbol.first);
	}

	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(roundTo(segmentSize, k) / k, 4);

	if (blockIdList.size() < k) {
		cerr << "Not enough blocks for decode " << blockIdList.size() << endl;
		exit(-1);
	}

	struct SegmentData segmentData;

	// copy segmentID from first available block
	segmentData.info.segmentId = blockData[blockIdList[0]].info.segmentId;
	segmentData.info.segLength = segmentSize;
	//segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	set<uint32_t> blockIdListSet(blockIdList.begin(), blockIdList.end());

	//if (blockIdList.size() != k + m) {
	int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
	char **data, **code;
	int *erasures;
	int j = 0;

	data = talloc<char*, uint32_t>(k);
	code = talloc<char*, uint32_t>(m);
	erasures = talloc<int, uint32_t>(k + m - blockIdList.size() + 1);

	for (uint32_t i = 0; i < k + m; i++) {
		i < k ? data[i] = talloc<char, uint32_t>(size) : code[i - k] =
			talloc<char, uint32_t>(size);
		if (blockIdListSet.count(i) > 0) {
            debug ("BlockData %d segment id = %" PRIu64 " block id = %" PRIu32 " blkSize = %" PRIu32 "\n",
                i, blockData[i].info.segmentId, blockData[i].info.blockId, blockData[i].info.blockSize); 
			i < k ? memcpy(data[i], blockData[i].buf, size) : memcpy(
					code[i - k], blockData[i].buf, size);
		} else {
			erasures[j++] = i;
		}
	}
	erasures[j] = -1;

	jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

	for (uint32_t i = 0; i < repairBlockIdList.size(); i++) {
		struct BlockData temp;
		temp.info.segmentId = segmentData.info.segmentId;
		//temp.info.blockId = erasures[i];
		temp.info.blockId = repairBlockIdList[i];
		temp.info.blockSize = size;

		temp.buf = MemoryPool::getInstance().poolMalloc(size);
		memcpy(temp.buf,
				(uint32_t) temp.info.blockId < k ?
				data[erasures[i]] : code[erasures[i] - k], size);

		blockData[temp.info.blockId] = temp;
		//blockData[erasures[i]] = temp;
		ret.push_back(temp);
	}

	// free memory
	for (uint32_t i = 0; i < k; i++) {
		tfree(data[i]);
	}
	tfree(data);

	for (uint32_t i = 0; i < m; i++) {
		tfree(code[i]);
	}
	tfree(code);

	free(matrix);
	tfree(erasures);
	//}

	return ret;
}

uint32_t RSCoding::getBlockCountFromSetting(string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	return k + m;
}

uint32_t RSCoding::getParityCountFromSetting(string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t m = params[1];
	return m;
}

uint32_t RSCoding::getBlockSize(uint32_t segmentSize, string setting) {
	vector<uint32_t> params = getParameters(setting);
	uint32_t k = params[0];
	return roundTo((roundTo(segmentSize, k) / k), 4);
}

vector<BlockData> RSCoding::computeDelta(BlockData oldBlock, BlockData newBlock,
        vector<offset_length_t> offsetLength, vector<uint32_t> parityVector) {

    const string codingSetting = newBlock.info.codingSetting;
    const uint32_t combinedLength = getCombinedLength(offsetLength);
	const vector<uint32_t> params = getParameters(codingSetting);
	const uint32_t k = params[0];

	// create a dummy blank segment with only the changed block data
	// oldBlockContainer: |0000 0000 AAAA 0000|
	// newBlockContainer: |0000 0000 BBBB 0000|

	SegmentData oldBlockContainer, newBlockContainer;
	uint32_t containerSize = combinedLength * k;
	oldBlockContainer.info.segLength = containerSize;
	newBlockContainer.info.segLength = containerSize;
	oldBlockContainer.buf = MemoryPool::getInstance().poolMalloc(containerSize);
	newBlockContainer.buf = MemoryPool::getInstance().poolMalloc(containerSize);

	memcpy (oldBlockContainer.buf + combinedLength * oldBlock.info.blockId, oldBlock.buf, combinedLength);
	memcpy (newBlockContainer.buf + combinedLength * newBlock.info.blockId, newBlock.buf, combinedLength);

	vector<BlockData> oldCodedBlocks = encode (oldBlockContainer, codingSetting);
	vector<BlockData> newCodedBlocks = encode (newBlockContainer, codingSetting);

	vector<BlockData> deltas (parityVector.size());

	for (int i = 0; i < (int) deltas.size(); i++) {
	    debug ("for i = %d, copying block id %" PRIu32 "\n", i, parityVector[i]);
	    BlockData &delta = deltas[i];
	    delta.info = oldBlock.info;
	    delta.buf = MemoryPool::getInstance().poolMalloc(combinedLength);
	    memcpy (delta.buf, oldCodedBlocks[parityVector[i]].buf, combinedLength);
	    bitwiseXor(delta.buf, delta.buf, newCodedBlocks[parityVector[i]].buf, combinedLength);
	}

    // cleanup
    for (int i = 0; i < (int)oldCodedBlocks.size(); i++) {
        debug ("Freeing %d\n", i);
        MemoryPool::getInstance().poolFree(oldCodedBlocks[i].buf);
        MemoryPool::getInstance().poolFree(newCodedBlocks[i].buf);
    }
    MemoryPool::getInstance().poolFree(oldBlockContainer.buf);
    MemoryPool::getInstance().poolFree(newBlockContainer.buf);

    return deltas;
}

//
// PRIVATE FUNCTION
//

vector<uint32_t> RSCoding::getParameters(string setting) {
	vector<uint32_t> params(3);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;
}
