#include <string.h>
#include <sstream>
#include <unordered_map>
#include <iostream>
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
//	const uint32_t k = params[1];
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
	//const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);
	
	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	vector<BlockData> RSBlockData;
	RSBlockData.reserve(rs_k + rs_m);

	debug("%s - %s\n", setting.c_str(), RSSetting.c_str());

	//TODO: Handle Node Failure
	
	unordered_map <uint32_t, uint32_t> base_id;

	unordered_map <uint32_t, struct BlockData> blockDataMap;

	base_id[0] = 0;
	for(uint32_t i = 1; i < n; ++i){
		base_id[i] = base_id[i-1] + blockGroupSize - (i - 1);
	}

	uint64_t segmentId = blockDataList[symbolList[0].first].info.segmentId;	

	for(uint32_t i = 0; i < symbolList.size(); ++i){
		uint32_t id = symbolList[i].first;	
		for(uint32_t j = 0; j < symbolList[i].second.size(); ++j){
			offset_length_t symbol = symbolList[i].second[j];
			uint32_t offset_id = symbol.first / symbolSize;
			struct BlockData blockData;
			if(offset_id >= id) {
				blockData.info.blockId = base_id[id] + offset_id - id;
			} else {
				uint32_t base_id2 = 0;
				for(uint32_t l = 0; l < offset_id; ++l) {
					base_id2 += n - 1 - l;
				}
				blockData.info.blockId = base_id2 + id - (offset_id + 1); 
			}
			blockData.info.segmentId = segmentId;
			blockData.info.blockSize = symbolSize;
			blockData.buf = blockDataList[id].buf + j * symbolSize;
			blockDataMap[blockData.info.blockId] = blockData;
		}
	}

	offset_length_t RSsymbol = make_pair(0, symbolSize);
	vector<offset_length_t> RSSymbolList = {RSsymbol};
	block_list_t RSRequiredBlockSymbols;
	uint32_t count = 0;
	for(uint32_t i = 0; i < rs_k + rs_m; ++i) {
		if(blockDataMap.count(i) == 1) {
			RSBlockData[i] = blockDataMap[i];
			symbol_list_t RSBlockSymbols = make_pair(i, RSSymbolList);
			RSRequiredBlockSymbols.push_back(RSBlockSymbols);
			++count;
		}
	}

	debug("%" PRIu32 "\n",count);

	return RSCoding::decode(RSBlockData, RSRequiredBlockSymbols, segmentSize, RSSetting);
}

block_list_t EMBRCoding::getRequiredBlockSymbols(vector<bool> blockStatus, uint32_t segmentSize, string setting){
	block_list_t requiredBlockSymbols;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	//const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);

	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	unordered_map< uint32_t, vector<offset_length_t> > symbolList;

	uint32_t count = 0;

	for(uint32_t i = 0; i < n; ++i) {
		if(count == rs_k) {
			if(symbolList[i].size() > 0) {
				symbol_list_t blockSymbols = make_pair(i, symbolList[i]);
				requiredBlockSymbols.push_back(blockSymbols);
			}
			continue;
		}

		if(blockStatus[i] == false) {
			offset_length_t symbol = make_pair(i * symbolSize, symbolSize);
			for(uint32_t j = i + 1; j < n; ++j) {
				if(blockStatus[j] == true) {
					symbolList[j].push_back(symbol);
					++count;
					if(count == rs_k) break;
				}
			}
		} else {
			for(uint32_t j = i; j < blockGroupSize; ++j) {
				offset_length_t symbol = make_pair(j * symbolSize, symbolSize);
				symbolList[i].push_back(symbol);
				++count;
				if(count == rs_k) break;
			}
		}

		if(symbolList[i].size() > 0) {
			symbol_list_t blockSymbols = make_pair(i, symbolList[i]);
			requiredBlockSymbols.push_back(blockSymbols);
		}
	}

	/*
	for(uint32_t i = 0; i < requiredBlockSymbols.size(); ++i) {
		debug("===== %" PRIu32 " =====\n",requiredBlockSymbols[i].first);
		for(uint32_t j = 0; j < requiredBlockSymbols[i].second.size(); ++j){
			debug("%" PRIu32 "-%" PRIu32 "\n",requiredBlockSymbols[i].second[j].first, requiredBlockSymbols[i].second[j].second);
		}
	}
	*/
	if(count < rs_k) {
		debug_error ("Not Enough Surviving Symbols, %" PRIu32 "/%" PRIu32 "\n",count,rs_k);
		exit(-1);
	}
	return requiredBlockSymbols;
}

/*
block_list_t EMBRCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks, vector<bool> blockStatus, uint32_t segmentSize, string setting){

}

vector<BlockData> EMBRCoding::repairBlocks(vector<uint32_t> repairBlockIdList, vector<BlockData> &blockData, block_list_t &symbolList, uint32_t segmentSize, string setting){

}
*/

uint32_t EMBRCoding::getBlockCountFromSetting (string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	return n;
}

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
