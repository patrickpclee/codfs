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

	return {};
}

SegmentData EvenOddCoding::decode(vector<BlockData> &blockDataList,
		block_list_t &symbolList, uint32_t segmentSize, string setting) {

	SegmentData segmentData;

	return segmentData;
}

block_list_t EvenOddCoding::getRequiredBlockSymbols(vector<bool> blockStatus,
		uint32_t segmentSize, string setting) {

	block_list_t requiredBlockList;
	return requiredBlockList;
}

block_list_t EvenOddCoding::getRepairBlockSymbols(vector<uint32_t> failedBlocks,
		vector<bool> blockStatus, uint32_t segmentSize, string setting) {

	block_list_t requiredBlockList;
	return requiredBlockList;
}

vector<BlockData> EvenOddCoding::repairBlocks(vector<uint32_t> repairBlockIdList,
		vector<BlockData> &blockData, block_list_t &symbolList,
		uint32_t segmentSize, string setting) {

	return {};
}

uint32_t EvenOddCoding::getBlockCountFromSetting(string setting) {
	return getParameters(setting) + 2;
}

//
// PRIVATE FUNCTION
//

uint32_t EvenOddCoding::getParameters(string setting) {
	uint32_t n;
	istringstream(setting) >> n;
	return n;
}
