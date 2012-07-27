#include <vector>
#include <string.h>
#include "dummydecode.hh"
#include "decodingbehaviour.hh"
#include "../osd/objectdata.hh"
#include "../osd/segmentdata.hh"
#include "../common/memorypool.hh"

using namespace std;

DummyDecode::DummyDecode() {

}

DummyDecode::~DummyDecode() {

}

struct ObjectData DummyDecode::decode(vector<struct SegmentData> segmentData) {
	// dummy decode: n = 1, k = 0
	// just copy data from segment to object

	struct ObjectData objectData;

	objectData.info.objectId = segmentData[0].info.objectId;
	objectData.info.objectSize = segmentData[0].info.segmentSize;

	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);

	memcpy(objectData.buf, segmentData[0].buf, objectData.info.objectSize);

	return objectData;
}
