#include <vector>
#include <string.h>
#include "raid0encode.hh"
#include "encodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Encode::Raid0Encode() {

}

Raid0Encode::~Raid0Encode() {

}

vector<struct SegmentData> Raid0Encode::encode(struct ObjectData objectData) {

	vector<struct SegmentData> segmentDataList;

	struct SegmentData segmentData;
	segmentData.info.objectId = objectData.info.objectId;
//	segmentData.info.offsetInObject = 0;
	segmentData.info.segmentId = 0;
	segmentData.info.segmentSize = objectData.info.objectSize;

	segmentData.buf = MemoryPool::getInstance().poolMalloc(
			segmentData.info.segmentSize);

	memcpy (segmentData.buf, objectData.buf, segmentData.info.segmentSize);

	segmentDataList.push_back(segmentData);
	return segmentDataList;
}
