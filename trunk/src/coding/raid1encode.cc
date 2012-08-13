#include <vector>
#include <string.h>
#include "raid1encode.hh"
#include "encodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid1Encode::Raid1Encode(uint32_t noOfReplications) {
	_noOfReplications = noOfReplications;
}

Raid1Encode::~Raid1Encode() {

}

vector<struct SegmentData> Raid1Encode::encode(struct ObjectData objectData) {

	vector<struct SegmentData> segmentDataList;

	for (uint32_t i = 0; i < _noOfReplications; i++) {

		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;
		segmentData.info.segmentSize = objectData.info.objectSize;

		// an optimization is to point the buf pointer to the same memory,
		// but it may create confusion when user wants to free the data

		segmentData.buf = MemoryPool::getInstance().poolMalloc(
				objectData.info.objectSize);

		memcpy(segmentData.buf, objectData.buf, segmentData.info.segmentSize);

		segmentDataList.push_back(segmentData);
	}

	return segmentDataList;
}
