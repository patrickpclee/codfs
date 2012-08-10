#include <vector>
#include <string.h>
#include "raid0decode.hh"
#include "decodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Decode::Raid0Decode() {

}

Raid0Decode::~Raid0Decode() {

}

struct ObjectData Raid0Decode::decode(vector<struct SegmentData> segmentData) {
	// raid0 decode: n = 1, k = 0
	// just copy data from segment to object

	struct ObjectData objectData;

	objectData.info.objectId = segmentData[0].info.objectId;
	objectData.info.objectSize = segmentData[0].info.segmentSize;

	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);

	memcpy(objectData.buf, segmentData[0].buf, objectData.info.objectSize);

	return objectData;
}
