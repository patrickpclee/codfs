/*
 * doCoding.hh
 */

#ifndef DOCODING_HH_
#define DOCODING_HH_

#include <vector>

using namespace std;

void doEncode(string srcSegmentPath);

void doDecode(uint64_t segmentId, uint64_t segmentSize, string dstSegmentPath,
		uint32_t numBlocks, vector<bool> blockStatus);

void doRepair(uint64_t segmentId, uint64_t segmentSize, uint32_t numBlocks,
		vector<bool> blockStatus, vector<string> dstBlockPaths);

#endif /* DOCODING_HH_ */
