#include <vector>
#include <string.h>
#include "raid0decode.hh"
#include "decodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Decode::Raid0Decode(uint32_t noOfStrips) {
	_noOfStrips = noOfStrips;
}

Raid0Decode::~Raid0Decode() {

}

struct ObjectData Raid0Decode::decode(vector<struct SegmentData> segmentData) {

	const uint32_t segmentCount = (uint32_t) segmentData.size();

	struct ObjectData objectData;

	// copy objectID from first segment
	objectData.info.objectId = segmentData[0].info.objectId;

	if (segmentCount == 1) {
		objectData.info.objectSize = segmentData[0].info.segmentSize;
	} else {
		// objectsize = first segment * (segmentCount - 1) + last segment
		objectData.info.objectSize = segmentData[0].info.segmentSize
				* (segmentCount - 1) + segmentData.end()->info.segmentSize;
	}

	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);

	for (struct SegmentData segment : segmentData) {
		memcpy(objectData.buf, segment.buf, segment.info.segmentSize);
	}

	return objectData;
}
