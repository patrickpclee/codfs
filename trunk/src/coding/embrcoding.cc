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
	
	vector<BlockData> RSBlockDataList = RSCoding::encode(segmentData, RSSetting);

	uint32_t symbolSize = RSBlockDataList[0].info.blockSize;
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
			memcpy(blockData.buf + offset * symbolSize, RSBlockDataList[dup_symbol].buf, symbolSize);
			dup_symbol += blockGroupSize - j - 1;
			++offset;
		}

		for(uint32_t j = 0; j < blockGroupSize - i; ++j) {
			memcpy(blockData.buf + offset * symbolSize, RSBlockDataList[symbol].buf, symbolSize); 
			++offset;
			++symbol;		
		}
	}

	for(uint32_t i = 0; i < RSBlockDataList.size(); ++i) {
		MemoryPool::getInstance().poolFree(RSBlockDataList[i].buf);
	}

	return blockDataList;
}

SegmentData EMBRCoding::decode(vector<BlockData> &blockDataList, block_list_t &symbolList, uint32_t segmentSize, string setting){
	string RSSetting;
	vector<BlockData> RSBlockDataList;
	block_list_t RSSymbolList;

	convertToRS(blockDataList, symbolList, segmentSize, setting, RSBlockDataList, RSSymbolList, RSSetting);

	return RSCoding::decode(RSBlockDataList, RSSymbolList, segmentSize, RSSetting);
}

void EMBRCoding::convertToRS(vector<BlockData> &blockDataList, block_list_t &symbolList, uint32_t segmentSize, string setting, vector<BlockData> &RSBlockDataList, block_list_t &RSSymbolList, string &RSSetting){
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	//const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);
	
	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	RSBlockDataList.reserve(rs_k + rs_m);

	//debug("%s - %s\n", setting.c_str(), RSSetting.c_str());

	//TODO: Handle Node Failure
	
	uint32_t base_id_v[n];
	uint32_t base_id_h[blockGroupSize];

	unordered_map <uint32_t, struct BlockData> blockDataMap;

	base_id_v[0] = 0;
	for(uint32_t i = 1; i < n; ++i)
		base_id_v[i] = base_id_v[i-1] + blockGroupSize - (i - 1);

	base_id_h[0] = 0;
	for(uint32_t i = 1; i < blockGroupSize; ++i) 
		base_id_h[i] = base_id_h[i - 1] + (n - i);

	uint64_t segmentId = blockDataList[symbolList[0].first].info.segmentId;	

	for(uint32_t i = 0; i < symbolList.size(); ++i){
		uint32_t id = symbolList[i].first;	
		for(uint32_t j = 0; j < symbolList[i].second.size(); ++j){
			offset_length_t symbol = symbolList[i].second[j];
			uint32_t offset_id = symbol.first / symbolSize;
			struct BlockData blockData;
			if(offset_id >= id) {
				blockData.info.blockId = base_id_v[id] + offset_id - id;
			} else {
				blockData.info.blockId = base_id_h[offset_id] + id - (offset_id + 1); 
			}
			blockData.info.segmentId = segmentId;
			blockData.info.blockSize = symbolSize;
			blockData.buf = blockDataList[id].buf + j * symbolSize;
			//debug("%" PRIu32 "-%" PRIu32 ":%" PRIu32 "\n", id,offset_id,blockData.info.blockId);
			blockDataMap[blockData.info.blockId] = blockData;
		}
	}

	offset_length_t RSsymbol = make_pair(0, symbolSize);
	vector<offset_length_t> RSSymbolList_ = {RSsymbol};
	uint32_t count = 0;
	for(uint32_t i = 0; i < rs_k + rs_m; ++i) {
		if(blockDataMap.count(i) == 1) {
			//debug("Added %" PRIu32 "\n", i);
			RSBlockDataList[i] = blockDataMap[i];
			symbol_list_t RSBlockSymbols = make_pair(i, RSSymbolList_);
			RSSymbolList.push_back(RSBlockSymbols);
			++count;
		}
	}

	//debug("%" PRIu32 "\n",count);
}

block_list_t EMBRCoding::getRequiredBlockSymbols(vector<bool> blockStatus, uint32_t segmentSize, string setting){
	block_list_t requiredBlockSymbols;
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	//const uint32_t k = params[1];
	//const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	//const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

//	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);

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

block_list_t EMBRCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks, vector<bool> blockStatus, uint32_t segmentSize, string setting){

	// More than One Node Failed, Use Decode Rountine
	if(failedBlocks.size() > 1) {
		return getRequiredBlockSymbols(blockStatus, segmentSize, setting);
	}

	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	//const uint32_t k = params[1];
	//const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	//const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);

	unordered_map< uint32_t, vector<offset_length_t> > symbolListMap;

	offset_length_t symbol;		
	for(uint32_t i = 0; i < failedBlocks.size(); ++i) {
		// Horizontal Group
		for(uint32_t j = 0; j < failedBlocks[i]; ++j) {
			if(blockStatus[j] == true){
				symbol = make_pair((failedBlocks[i] - 1)* symbolSize, symbolSize);
				symbolListMap[j].push_back(symbol);
			}
		}
		// Vertical Group
		for(uint32_t j = failedBlocks[i]; j < blockGroupSize; ++j) {
			if(blockStatus[j + 1] == true){
				symbol = make_pair(failedBlocks[i] * symbolSize, symbolSize);
				symbolListMap[j + 1].push_back(symbol);
			}
		}
	}

	block_list_t requiredBlockSymbols;
	symbol_list_t blockSymbols;
	for(auto symbolList: symbolListMap) {
		blockSymbols = make_pair(symbolList.first, symbolList.second);
		requiredBlockSymbols.push_back(blockSymbols);
	}

	return requiredBlockSymbols;
}

vector<BlockData> EMBRCoding::repairBlocks(vector<uint32_t> repairBlockIdList, vector<BlockData> &blockDataList, block_list_t &symbolList, uint32_t segmentSize, string setting){

	//debug ("Repair Setting = %s\n", setting.c_str());

	vector<uint32_t> params = getParameters(setting);

	//const uint32_t rs_k = k * (n - 1) - k * (k - 1) / 2;
	//const uint32_t rs_m = n * (n - 1) / 2 - k * (2 * n - k - 1) / 2;

	const uint32_t n = params[0];
	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;
	const uint32_t symbolSize = roundTo(roundTo(segmentSize, rs_k) / rs_k, 4);
	const uint32_t blockSize = symbolSize * blockGroupSize;

	string RSSetting;
	vector<BlockData> RSBlockDataList;
	block_list_t RSSymbolList;

	convertToRS(blockDataList, symbolList, segmentSize, setting, RSBlockDataList, RSSymbolList, RSSetting);
	uint32_t RSBlockCount = RSCoding::getBlockCountFromSetting(RSSetting);

	bool data_availability[RSBlockCount];
	std::fill_n(data_availability, RSBlockCount, false);
	// Build Data Availability List
	for(auto blockSymbol : RSSymbolList) {
		data_availability[blockSymbol.first] = true;
	}

	/*
	for(uint32_t i = 0; i < RSBlockCount; ++i) {
		debug("%" PRIu32 "-%d\n", i, data_availability[i]);
	}
	*/

	vector<uint32_t> failedBlocks;
	// Build RS Repair Block List
	for(uint32_t i = 0; i < RSBlockCount; ++i) {
		if(data_availability[i] == false)
			failedBlocks.push_back(i);
	}

	// Use RS Repair
	if(repairBlockIdList.size() > 1) {
		vector<BlockData> RSRecoverBlockDataList;
		RSRecoverBlockDataList = RSCoding::repairBlocks(failedBlocks, RSBlockDataList, RSSymbolList, segmentSize, RSSetting);
		for(uint32_t i = 0; i < RSRecoverBlockDataList.size(); ++i)
			RSBlockDataList[RSRecoverBlockDataList[i].info.blockId] = RSRecoverBlockDataList[i];
	}

	uint32_t base_id_v[n];
	uint32_t base_id_h[blockGroupSize];
	base_id_v[0] = 0;
	for(uint32_t i = 1; i < n; ++i)
		base_id_v[i] = base_id_v[i-1] + blockGroupSize - (i - 1);

	base_id_h[0] = 0;
	for(uint32_t i = 1; i < blockGroupSize; ++i) 
		base_id_h[i] = base_id_h[i - 1] + (n - i);

	uint64_t segmentId = blockDataList[symbolList[0].first].info.segmentId;	
	vector<struct BlockData> repairedBlockData;
	struct BlockData blockData;
	for(uint32_t i = 0; i < repairBlockIdList.size(); ++i) {
		uint32_t id = repairBlockIdList[i];
		blockData.info.segmentId = segmentId;
		blockData.info.blockId = repairBlockIdList[i];
		blockData.info.blockSize = blockSize; 
		blockData.buf = MemoryPool::getInstance().poolMalloc(blockSize);

		uint32_t RSBlockId;

		for(uint32_t j = 0; j < blockGroupSize; ++j) {

			if(j >= id) 
				// Vertical Group
				RSBlockId = base_id_v[id] + j - id;
			else
				// Horizontal Group
				RSBlockId = base_id_h[j] + id - (j + 1);
		
			debug("%" PRIu32 "-%" PRIu32 " <- %" PRIu32 "\n", id, j, RSBlockId);
			memcpy(blockData.buf + j * symbolSize, RSBlockDataList[RSBlockId].buf, symbolSize);
		}

		blockDataList[repairBlockIdList[i]] = blockData;
		repairedBlockData.push_back(blockData);
	}

	return repairedBlockData;
}

uint32_t EMBRCoding::getBlockCountFromSetting (string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	return n;
}

uint32_t EMBRCoding::getBlockSize(uint32_t segmentSize, string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
//	const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = params[3];
	const uint32_t rs_m = params[4];
	const uint32_t rs_w = w;

	const uint32_t blockGroupSize = (rs_k + rs_m) * 2 / n;

	string RSSetting = RSCoding::generateSetting(rs_k, rs_m, rs_w);

	uint32_t symbolSize = RSCoding::getBlockSize(segmentSize, RSSetting);

	return symbolSize * blockGroupSize;
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
