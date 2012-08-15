#include <sstream>
#include <iostream>
#include "coding.hh"
#include "raid1coding.hh"
#include "../common/memorypool.hh"

Raid1Coding::Raid1Coding() {

}

Raid1Coding::~Raid1Coding() {

}

vector<struct SegmentData> Raid1Coding::encode(struct ObjectData objectData, string setting) {

	const uint32_t noOfReplications = getNoOfReplications(setting);
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

struct ObjectData Raid1Coding::decode(vector<struct SegmentData> segmentData, string setting) {

	struct ObjectData objectData;
	objectData.info.objectId = segmentData[0].info.objectId;
	objectData.info.objectSize = segmentData[0].info.segmentSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);
	memcpy (objectData.buf, segmentData[0].buf, objectData.info.objectSize);

	return objectData;
}

uint32_t Raid1Coding::getNoOfReplications(string setting) {
	uint32_t noOfReplications;
	istringstream(setting) >> noOfReplications;
	return noOfReplications;
}

/*
string Raid1Coding::generateSetting (int noOfReplications) {
	return to_string (noOfReplications);
}
*/

void Raid1Coding::display() {

}
