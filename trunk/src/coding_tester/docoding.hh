/*
 * doCoding.hh
 */

#ifndef DOCODING_HH_
#define DOCODING_HH_

#include <vector>

void doEncode(std::string srcSegmentPath);
void doDecode(uint64_t segmentId, uint64_t segmentSize, std::string dstSegmentPath,
		uint32_t numBlocks, std::vector<bool> secondaryOsdStatus);

#endif /* DOCODING_HH_ */
