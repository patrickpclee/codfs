/*
 * doCoding.hh
 */

#ifndef DOCODING_HH_
#define DOCODING_HH_

#include <vector>

void doEncode(std::string srcObjectPath);
void doDecode(uint64_t objectId, uint64_t objectSize, std::string dstObjectPath,
		uint32_t numSegments, std::vector<bool> secondaryOsdStatus);

#endif /* DOCODING_HH_ */
