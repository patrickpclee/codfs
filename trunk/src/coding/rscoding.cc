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

vector<struct BlockData> RSCoding::encode(struct SegmentData segmentData,
		string setting) {

	vector<struct BlockData> blockDataList;
	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo((roundTo(segmentData.info.segmentSize, k) / k),4);

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
		if(i == k-1){
			//memset(blockData.buf, 0, size);
			memcpy(blockData.buf, bufPos, segmentData.info.segmentSize - i*size);
			memset(blockData.buf + segmentData.info.segmentSize - i*size, 0, k*size - segmentData.info.segmentSize);
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

struct SegmentData RSCoding::decode(vector<struct BlockData> &blockData,
		vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
		string setting) {

	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(roundTo(segmentSize, k) / k, 4);

	if (requiredBlocks.size() < k) {
		cerr << "Not enough blocks for decode " << requiredBlocks.size()
				<< endl;
		exit(-1);
	}

	struct SegmentData segmentData;

	// copy segmentID from first available block
	segmentData.info.segmentId = blockData[requiredBlocks[0]].info.segmentId;
	segmentData.info.segmentSize = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	set<uint32_t> requiredBlocksSet(requiredBlocks.begin(),
			requiredBlocks.end());

	if (requiredBlocks.size() != k + m) {
		int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
		char **data, **code;
		int *erasures;
		int j = 0;

		data = talloc<char*, uint32_t>(k);
		code = talloc<char*, uint32_t>(m);
		erasures = talloc<int, uint32_t>(k + m - requiredBlocks.size() + 1);

		for (uint32_t i = 0; i < k + m; i++) {
			i < k ? data[i] = talloc<char, uint32_t>(size) : code[i - k] =
							talloc<char, uint32_t>(size);
			if (requiredBlocksSet.count(i) > 0) {
				i < k ? memcpy(data[i], blockData[i].buf, size) : memcpy(
								code[i - k], blockData[i].buf, size);
			} else {
				erasures[j++] = i;
			}
		}
		erasures[j] = -1;

		jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

		for (uint32_t i = 0; i < k + m - requiredBlocks.size(); i++) {
			struct BlockData temp;
			temp.info.segmentId = segmentData.info.segmentId;
			temp.info.blockId = erasures[i];
			temp.info.blockSize = size;

			temp.buf = MemoryPool::getInstance().poolMalloc(size);
			memcpy(temp.buf,
					(uint32_t) erasures[i] < k ?
							data[erasures[i]] : code[erasures[i] - k], size);

			blockData[erasures[i]] = temp;
		}

		for (uint32_t i = 0; i < m; i++) {
			MemoryPool::getInstance().poolFree(blockData[k + i].buf);
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

	requiredBlocks.clear();
	for (uint32_t i = 0; i < k; i++) {
		requiredBlocks.push_back(i);
	}

	uint64_t offset = 0;
	for (uint32_t i = 0; i < k - 1; i++) {
		memcpy(segmentData.buf + offset, blockData[i].buf,
				blockData[i].info.blockSize);
		offset += blockData[i].info.blockSize;
	}
	memcpy(segmentData.buf + offset, blockData[k - 1].buf, segmentSize - offset);

	return segmentData;
}

vector<uint32_t> RSCoding::getRequiredBlockIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	// for raid 5, only requires n-1 stripes (noOfDataStripes) to decode
	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t noOfDataStripes = k + m;
	vector<uint32_t> requiredBlocks;
	requiredBlocks.reserve(noOfDataStripes);

	if ((uint32_t) failedOsdCount > m) {
		return {};
	}

	for (uint32_t i = 0; i < noOfDataStripes; i++) {
		if (secondaryOsdStatus[i] != false) {
			requiredBlocks.push_back(i);
		}
	}

	return requiredBlocks;
}

vector<uint32_t> RSCoding::getRepairSrcBlockIds(string setting,
		vector<uint32_t> failedBlocks, vector<bool> blockStatus) {

	return {};
}

vector<struct BlockData> RSCoding::repairBlocks(
		vector<uint32_t> failedBlocks,
		vector<struct BlockData> &repairSrcBlocks,
		vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
		string setting) {

	return {};
}

//
// PRIVATE FUNCTION
//

vector<uint32_t> RSCoding::getParams(string setting) {
	vector<uint32_t> params(3);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;
}
