#include <vector>
#include <string.h>
#include "dummyencode.hh"
#include "encodingbehaviour.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

DummyEncode::DummyEncode() {

}

DummyEncode::~DummyEncode() {

}

vector<struct SegmentData> DummyEncode::encode(struct ObjectData objectData) {
	// dummy encode: n = 1, k = 0
	// just copy data from object to segment

	vector<struct SegmentData> segmentDataList;

	struct SegmentData segmentData;
	segmentData.info.objectId = objectData.info.objectId;
	segmentData.info.offsetInObject = 0;
	segmentData.info.segmentId = 0;
	segmentData.info.segmentSize = objectData.info.objectSize;

	segmentData.buf = MemoryPool::getInstance().poolMalloc(
			segmentData.info.segmentSize);

	memcpy (segmentData.buf, objectData.buf, segmentData.info.segmentSize);

	segmentDataList.push_back(segmentData);
	return segmentDataList;
}
