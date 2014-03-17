#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "cauchycoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/cauchy.h"

}

using namespace std;

CauchyCoding::CauchyCoding() {

}

CauchyCoding::~CauchyCoding() {

}

vector<BlockData> CauchyCoding::encode(SegmentData segmentData, string setting) {

	vector<struct BlockData> blockDataList;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(
			(roundTo(segmentData.info.segLength, k*w) / (k*w)), 4);


	if (k <= 0 || m < 0 || w <= 0 || w > 32
			|| (w < 30 && k + m > ((uint32_t) 1 << w))) {
		cerr << "Bad Parameters" << endl;
		exit(-1);
	}

	int *matrix = cauchy_good_general_coding_matrix(k, m, w);
	int *bitmatrix = jerasure_matrix_to_bitmatrix(k, m, w, matrix);
	int **smart = jerasure_smart_bitmatrix_to_schedule(k, m, w, bitmatrix);

	char **data, **code;
	data = talloc<char*, uint32_t>(k);

	for (uint32_t i = 0; i < k; i++) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = size*w;
		blockData.buf = MemoryPool::getInstance().poolMalloc(size*w);
		char* bufPos = segmentData.buf + i * size*w;
		if (i * w *size >= segmentData.info.segLength) {
			//Zero Padding
		} else if ((i + 1) * w * size > segmentData.info.segLength) {
			memcpy(blockData.buf, bufPos,
					segmentData.info.segLength - i * w * size);
			memset(blockData.buf + segmentData.info.segLength - i * w * size, 0,
					(i + 1) * w * size - segmentData.info.segLength);
		} else
			memcpy(blockData.buf, bufPos, w * size);

		data[i] = blockData.buf;

		blockDataList.push_back(blockData);
	}

	if (m == 0){
		tfree(data);
		return blockDataList;
	}

	code = talloc<char*, uint32_t>(m);
	for (uint32_t i = 0; i < m; i++) {
		code[i] = talloc<char, uint32_t>(size*w);
	}

	jerasure_schedule_encode(k, m, w, smart, data, code, w*size, size);

	//	for (uint32_t i = 0; i < k; i++) {
	//		char path[100];
	//		sprintf (path, "encode %" PRIu32, i);
	//		FILE* f = fopen (path, "w");
	//		fwrite (data[i], w*size, 1,f);
	//	}
	//	for (uint32_t i = 0; i < m; i++) {
	//		char path[100];
	//		sprintf (path, "encode %" PRIu32, i+k);
	//		FILE* f = fopen (path, "w");
	//		fwrite (code[i], w*size, 1,f);
	//	}

	for (uint32_t i = 0; i < m; i++) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = k + i;
		blockData.info.blockSize = size*w;

		blockData.buf = code[i];
		//		blockData.buf = MemoryPool::getInstance().poolMalloc(size*w);
		//		memcpy(blockData.buf, code[i], size*w);

		blockDataList.push_back(blockData);
	}

	//	for (uint32_t i = 0; i < k+m; i++) {
	//		char path[100];
	//		sprintf (path, "ENCODE %" PRIu32, i);
	//		FILE* f = fopen (path, "w");
	//		fwrite (blockDataList[i].buf, blockDataList[i].info.blockSize, 1,f);
	//	}
	//	for (uint32_t i = 0; i < k+m; i++) {
	//			char path[100];
	//			sprintf (path, "ENCODE %" PRIu32, i);
	//			FILE* f = fopen (path, "w");
	//			fwrite (blockDataList[i].buf, blockDataList[i].info.blockSize, 1, f);
	//		}

	// free memory
	//for (uint32_t i = 0; i < k; i++) {
	//	tfree(data[i]);
	//}
	tfree(data);

	//for (uint32_t i = 0; i < m; i++) {
	//	tfree(code[i]);
	//}
	tfree(code);

	return blockDataList;
}

SegmentData CauchyCoding::decode(vector<BlockData> &blockDataList,
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
	const uint32_t size = roundTo(roundTo(segmentSize, k*w) / (k*w), 4);

	if (blockIdList.size() < k) {
		cerr << "Not enough blocks for decode " << blockIdList.size() << endl;
		exit(-1);
	}
	//
	//	for (uint32_t i = 0; i < k+m; i++) {
	//		char path[100];
	//		sprintf (path, "DECODE %" PRIu32, i);
	//		FILE* f = fopen (path, "w");
	//		fwrite (blockDataList[i].buf, blockDataList[i].info.blockSize, 1,f);
	//	}

	struct SegmentData segmentData;

	// copy segmentID from first available block
	segmentData.info.segmentId = blockDataList[blockIdList[0]].info.segmentId;
	segmentData.info.segLength = segmentSize;
	segmentData.buf = MemoryPool::getInstance().poolMalloc(segmentSize);

	set<uint32_t> blockIdListSet(blockIdList.begin(), blockIdList.end());

	int *matrix = cauchy_good_general_coding_matrix(k, m, w);
	int *bitmatrix = jerasure_matrix_to_bitmatrix(k, m, w, matrix);
	//		int **smart = jerasure_smart_bitmatrix_to_schedule(k, m, w, bitmatrix);

	char **data, **code;
	int *erasures;
	int j = 0;

	data = talloc<char*, uint32_t>(k);
	code = talloc<char*, uint32_t>(m);
	erasures = talloc<int, uint32_t>(k + m - blockIdList.size() + 1);

	for (uint32_t i = 0; i < k + m; i++) {
		i < k ? data[i] = talloc<char, uint32_t>(size*w) : code[i - k] =
			talloc<char, uint32_t>(size*w);
		if (blockIdListSet.count(i) > 0) {
			i < k ? memcpy(data[i], blockDataList[i].buf, size*w) : memcpy(
					code[i - k], blockDataList[i].buf, size*w);
		} else {
			erasures[j++] = i;
		}
	}
	erasures[j] = -1;


	//		for (uint32_t i = 0; i < k; i++) {
	//			char path[100];
	//			sprintf (path, "predecode %" PRIu32, i);
	//			FILE* f = fopen (path, "w");
	//			fwrite (data[i], w*size, 1,f);
	//		}
	//		for (uint32_t i = 0; i < m; i++) {
	//					char path[100];
	//					sprintf (path, "predecode %" PRIu32, i+k);
	//					FILE* f = fopen (path, "w");
	//					fwrite (code[i], w*size, 1,f);
	//				}
	jerasure_schedule_decode_lazy(k, m, w, bitmatrix, erasures, data, code, w*size, size, 1);

	//	for (uint32_t i = 0; i < k+m; i++) {
	//		char path[100];
	//		sprintf (path, "DECODE %" PRIu32, i);
	//		FILE* f = fopen (path, "w");
	//		fwrite (blockDataList[i].buf, blockDataList[i].info.blockSize, 1,f);
	//	}

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

block_list_t CauchyCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(blockStatus.begin(), blockStatus.end(),
			false);

	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	//const uint32_t noOfDataStripes = k + m;
	const uint32_t noOfDataStripes = k*w;
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
		offset_length_t symbol = make_pair(0, blockSize*w);
		vector<offset_length_t> symbolList = { symbol };
		symbol_list_t blockSymbols = make_pair(i, symbolList);
		requiredBlockSymbols.push_back(blockSymbols);
	}

	return requiredBlockSymbols;
}

block_list_t CauchyCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	// for raid-6, same as download
	return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
}

vector<BlockData> CauchyCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	//	string blockIdString;
	//	for (auto block : repairBlockIdList) {
	//		blockIdString += block + " ";
	//	}
	//
	//	debug_yellow("Start repairBlocks for %s\n", blockIdString.c_str());

	//set<uint32_t> repairBlockIdListSet(repairBlockIdList.begin(), repairBlockIdList.end());
	vector<BlockData> ret;
	//decode(blockData, symbolList, segmentSize, setting);
	vector<uint32_t> blockIdList;
	for (auto blockSymbol : symbolList) {
		blockIdList.push_back(blockSymbol.first);
	}

	set<uint32_t> repairListSet(repairBlockIdList.begin(), repairBlockIdList.end());

	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(roundTo(segmentSize, k*w) / (k*w), 4);

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
	int *matrix = cauchy_good_general_coding_matrix(k, m, w);
	int *bitmatrix = jerasure_matrix_to_bitmatrix(k, m, w, matrix);
	//	int **smart = jerasure_smart_bitmatrix_to_schedule(k, m, w, bitmatrix);

	char **data, **code;
	int *erasures;
	int j = 0;

	data = talloc<char*, uint32_t>(k);
	code = talloc<char*, uint32_t>(m);
	erasures = talloc<int, uint32_t>(k + m - blockIdList.size() + 1);

	for (uint32_t i = 0; i < k + m; i++) {
		i < k ? data[i] = talloc<char, uint32_t>(size*w) : code[i - k] =
			talloc<char, uint32_t>(size*w);
		if (blockIdListSet.count(i) > 0) {
			i < k ? memcpy(data[i], blockData[i].buf, size*w) : memcpy(
					code[i - k], blockData[i].buf, size*w);
		} else {
			erasures[j++] = i;
		}
	}
	erasures[j] = -1;

	jerasure_schedule_decode_lazy(k, m, w, bitmatrix, erasures, data, code, w*size, size, 1);

	for (uint32_t i = 0; i < repairBlockIdList.size(); i++) {
		struct BlockData temp;
		temp.info.segmentId = segmentData.info.segmentId;
		//temp.info.blockId = erasures[i];
		temp.info.blockId = repairBlockIdList[i];
		temp.info.blockSize = size*w;

		temp.buf = MemoryPool::getInstance().poolMalloc(size*w);
		memcpy(temp.buf,
				(uint32_t) erasures[i] < k ?
				data[erasures[i]] : code[erasures[i] - k], size*w);

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

uint32_t CauchyCoding::getBlockCountFromSetting(string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	return k + m;
}

uint32_t CauchyCoding::getParityCountFromSetting(string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t m = params[1];
	return m;
}

uint32_t CauchyCoding::getBlockSize(uint32_t segmentSize, string setting) {
	vector<uint32_t> params = getParameters(setting);
	uint32_t k = params[0];
	uint32_t w = params[2];
	return roundTo((roundTo(segmentSize, k*w) / (k*w)), 4) *w;
}

//
// PRIVATE FUNCTION
//

vector<uint32_t> CauchyCoding::getParameters(string setting) {
	vector<uint32_t> params(3);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;

}
