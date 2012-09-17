#include <sstream>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "../common/debug.hh"
#include "coding.hh"
#include "raid1coding.hh"
#include "../common/memorypool.hh"

Raid1Coding::Raid1Coding() {

}

Raid1Coding::~Raid1Coding() {

}

vector<struct SegmentData> Raid1Coding::encode(struct ObjectData objectData,
		string setting) {

	const uint32_t raid1_n = getParameters(setting);
	vector<struct SegmentData> segmentDataList;

	debug("RAID1: Replication No = %" PRIu32 "\n", raid1_n);

	for (uint32_t i = 0; i < raid1_n; i++) {

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

struct ObjectData Raid1Coding::decode(vector<struct SegmentData> &segmentData,
		vector<uint32_t> &requiredSegments,  uint32_t objectSize, string setting) {

	// for raid1, only use first required segment to decode
	uint32_t segmentId = requiredSegments[0];

	struct ObjectData objectData;
	objectData.info.objectId = segmentData[segmentId].info.objectId;
	objectData.info.objectSize = objectSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(
			objectData.info.objectSize);
	memcpy(objectData.buf, segmentData[segmentId].buf, objectData.info.objectSize);

	return objectData;
}

uint32_t Raid1Coding::getParameters(string setting) {
	uint32_t raid1_n;
	istringstream(setting) >> raid1_n;
	return raid1_n;
}

vector<uint32_t> Raid1Coding::getRequiredSegmentIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// for Raid1 Coding, find the first running OSD
	vector<bool>::iterator it;
	it = find(secondaryOsdStatus.begin(), secondaryOsdStatus.end(), true);

	// not found (no OSD is running)
	if (it == secondaryOsdStatus.end()) {
		return {};
	}

	// return the index
	uint32_t offset = it - secondaryOsdStatus.begin();
	return {offset};
}
