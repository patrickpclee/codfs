#include <string.h>
#include <sstream>
#include "coding.hh"
#include "embrcoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

EMBRCoding::EMBRCoding(){

}

EMBRCoding::~EMBRCoding(){

}

vector<BlockData> EMBRCoding::encode(SegmentData segmentData, string setting) {
	vector<struct BlockData> blockDataList;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);
	
	vector<BlockData> RSBlockData = RSCoding::encode(segmentData, RSSetting);

	uint32_t symbolSize = RSBlockData[0].info.blockSize;
	uint32_t blockSize = symbolSize * blockGroupSize;

	uint32_t symbol = 0;
	uint32_t dup_symbol;
	uint32_t offset = 0;
	for(uint32_t i = 0; i < n; ++i) {
		struct BlockData blockData;
		blockData.info.segmentId = segmentData.info.segmentId;
		blockData.info.blockId = i;
		blockData.info.blockSize = blockSize;
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);
		blockDataList.push_back(blockData);

		offset = 0;

		dup_symbol = i - 1;
		for(uint32_t j = 0; j < i; ++j) {
			memcpy(blockData.buf + offset * symbolSize, RSBlockData[dup_symbol].buf, symbolSize);
			dup_symbol += blockGroupSize - j - 1;
			++offset;
		}

		for(uint32_t j = 0; j < blockGroupSize - i; ++j) {
			memcpy(blockData.buf + offset * symbolSize, RSBlockData[symbol].buf, symbolSize); 
			++offset;
			++symbol;		
		}
	}

	return blockDataList;
}

SegmentData EMBRCoding::decode(vector<BlockData> &blockDataList, block_list_t &symbolList, uint32_t segmentSize, string setting){
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);
	
	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	vector<BlockData> RSBlockData;

	//TODO: Handle Node Failure
	
	uint32_t symbol = 0;
	block_list_t RSBlockSymbols;
	for(uint32_t i = 0; i < k; ++i){
		for(uint32_t j = 0; j < symbolList[i].second.size(); ++j){
			struct BlockData blockData;
			blockData.info.segmentId = blockDataList[0].info.segmentId;
			blockData.info.blockId = symbol;
			blockData.info.blockSize = symbolSize;
			blockData.buf = blockDataList[i].buf + symbolList[i].second[j].first;
			RSBlockData.push_back(blockData);

			offset_length_t RSSymbol = make_pair(0, symbolSize);
			vector<offset_length_t> symbolList = {RSSymbol};
			symbol_list_t blockSymbols = make_pair(symbol, symbolList);
			RSBlockSymbols.push_back(blockSymbols);

			++symbol;
		}
		
	}

	return RSCoding::decode(RSBlockData, RSBlockSymbols, segmentSize, RSSetting);
}

block_list_t EMBRCoding::getRequiredBlockSymbols(vector<bool> blockStatus, uint32_t segmentSize, string setting){
	block_list_t requiredBlockSymbols;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);

	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	// TODO: Handle Node Failure
	
	uint32_t offset = 0;
	for(uint32_t i = 0; i < k; ++i) {
		vector<offset_length_t> symbolList = {};
		offset = i * symbolSize;
		for(uint32_t j = 0; j < blockGroupSize - i; ++j) {
			offset_length_t symbol = make_pair(offset,symbolSize);
			symbolList.push_back(symbol);
			offset += symbolSize;
		}
		symbol_list_t blockSymbols = make_pair(i, symbolList);
		requiredBlockSymbols.push_back(blockSymbols);
	}

	return requiredBlockSymbols;
}

/*
block_list_t EMBRCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks, vector<bool> blockStatus, uint32_t segmentSize, string setting){

}

vector<BlockData> EMBRCoding::repairBlocks(vector<uint32_t> repairBlockIdList, vector<BlockData> &blockData, block_list_t &symbolList, uint32_t segmentSize, string setting){

}
*/

//
// PRIVATE FUNCTION
//

vector<uint32_t> EMBRCoding::getParameters(string setting) {
	vector<uint32_t> params(5);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;
}
