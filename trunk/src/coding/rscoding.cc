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
			(roundTo(segmentData.info.segmentSize, k) / k), 4);

	if (k <= 0 || m <= 0 || (w != 8 && w != 16 && w != 32)
			|| (w <= 16 && k + m > (1 << w))) {
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
		if (i == k - 1) {
			//memset(blockData.buf, 0, size);
			memcpy(blockData.buf, bufPos,
					segmentData.info.segmentSize - i * size);
			memset(blockData.buf + segmentData.info.segmentSize - i * size, 0,
					k * size - segmentData.info.segmentSize);
		} else
			memcpy(blockData.buf, bufPos, size);

		blockDataList.push_back(blockData);

		data[i] = talloc<char, uint32_t>(size);
		memcpy(data[i], blockData.buf, size);
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

		blockData.buf = MemoryPool::getInstance().poolMalloc(size);
		memcpy(blockData.buf, code[i], size);

		blockDataList.push_back(blockData);
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

	return blockDataList;
}

SegmentData RSCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	// transform symbolList to blockIdList
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
	segmentData.info.segmentId = blockDataList[blockIdList[0]].info.segmentId;
	segmentData.info.segmentSize = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	set<uint32_t> blockIdListSet(blockIdList.begin(), blockIdList.end());

	if (blockIdList.size() != k + m) {
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
				i < k ? memcpy(data[i], blockDataList[i].buf, size) : memcpy(
								code[i - k], blockDataList[i].buf, size);
			} else {
				erasures[j++] = i;
			}
		}
		erasures[j] = -1;

		jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

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

		for (uint32_t i = 0; i < m; i++) {
			MemoryPool::getInstance().poolFree(blockDataList[k + i].buf);
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

	}

	blockIdList.clear();
	for (uint32_t i = 0; i < k; i++) {
		blockIdList.push_back(i);
	}

	uint64_t offset = 0;
	for (uint32_t i = 0; i < k - 1; i++) {
		memcpy(segmentData.buf + offset, blockDataList[i].buf,
				blockDataList[i].info.blockSize);
		offset += blockDataList[i].info.blockSize;
	}
	memcpy(segmentData.buf + offset, blockDataList[k - 1].buf,
			segmentSize - offset);

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
	const uint32_t size = roundTo(roundTo(segmentSize, noOfDataStripes) / noOfDataStripes, 4);
	vector<uint32_t> requiredBlocks;
	requiredBlocks.reserve(noOfDataStripes);

	if ((uint32_t) failedOsdCount > m) {
		return {};
	}

	for (uint32_t i = 0; i < noOfDataStripes; i++) {
		if (blockStatus[i] != false) {
			requiredBlocks.push_back(i);
		}
	}

	uint32_t blockSize = roundTo(segmentSize, noOfDataStripes)
			/ noOfDataStripes;
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
		segmentData.info.segmentSize = segmentSize;
		segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

		set<uint32_t> blockIdListSet(blockIdList.begin(), blockIdList.end());

		if (blockIdList.size() != k + m) {
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
					i < k ? memcpy(data[i], blockData[i].buf, size) : memcpy(
									code[i - k], blockData[i].buf, size);
				} else {
					erasures[j++] = i;
				}
			}
			erasures[j] = -1;

			jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

			for (uint32_t i = 0; i < k + m - blockIdList.size(); i++) {
				struct BlockData temp;
				temp.info.segmentId = segmentData.info.segmentId;
				temp.info.blockId = erasures[i];
				temp.info.blockSize = size;

				temp.buf = MemoryPool::getInstance().poolMalloc(size);
				memcpy(temp.buf,
						(uint32_t) erasures[i] < k ?
								data[erasures[i]] : code[erasures[i] - k], size);

				blockData[erasures[i]] = temp;
				ret.push_back(temp);
			}

//			for (uint32_t i = 0; i < m; i++) {
//				MemoryPool::getInstance().poolFree(blockData[k + i].buf);
//			}

			// free memory
			for (uint32_t i = 0; i < k; i++) {
				tfree(data[i]);
			}
			tfree(data);

			for (uint32_t i = 0; i < m; i++) {
				tfree(code[i]);
			}
			tfree(code);

		}

	return ret;
}

uint32_t RSCoding::getBlockCountFromSetting(string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	return k + m;
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
