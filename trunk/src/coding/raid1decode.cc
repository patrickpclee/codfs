#include <vector>
#include <string.h>
#include "raid1decode.hh"
#include "decodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid1Decode::Raid1Decode(uint32_t noOfReplications) {
	_noOfReplications = noOfReplications;
}

Raid1Decode::~Raid1Decode() {

}

struct ObjectData Raid1Decode::decode(vector<struct SegmentData> segmentData) {

	struct ObjectData objectData;
	objectData.info.objectId = segmentData[0].info.objectId;
	objectData.info.objectSize = segmentData[0].info.segmentSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);
	memcpy (objectData.buf, segmentData[0].buf, objectData.info.objectSize);

	return objectData;
}
