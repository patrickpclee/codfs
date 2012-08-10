#include <vector>
#include <string.h>
#include "raid0encode.hh"
#include "encodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Encode::Raid0Encode(uint32_t noOfStrips) {
	_noOfStrips = noOfStrips;
}

Raid0Encode::~Raid0Encode() {

}

uint32_t roundTo(unsigned int value, unsigned int roundTo) {
	return ((value + roundTo - 1) / roundTo) * roundTo;
}

vector<struct SegmentData> Raid0Encode::encode(struct ObjectData objectData) {

	vector<struct SegmentData> segmentDataList;

	// calculate size of each strip
	const uint32_t stripSize = roundTo(objectData.info.objectSize, _noOfStrips)
			/ _noOfStrips;

	for (uint32_t i = 0; i < _noOfStrips; i++) {

		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;

		if (i == _noOfStrips - 1) { // last segment
			segmentData.info.segmentSize = objectData.info.objectSize - i * stripSize;

		} else {
			segmentData.info.segmentSize = stripSize;
		}

		segmentData.buf = MemoryPool::getInstance().poolMalloc(stripSize);

		char* bufPos = objectData.buf + i * stripSize;

		memcpy(segmentData.buf, bufPos, segmentData.info.segmentSize);

		segmentDataList.push_back(segmentData);
	}

	return segmentDataList;
}
