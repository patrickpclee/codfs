#include <sstream>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "coding.hh"
#include "raid0coding.hh"
#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid0Coding::Raid0Coding() {

}

Raid0Coding::~Raid0Coding() {

}

vector<struct SegmentData> Raid0Coding::encode(struct ObjectData objectData,
		string setting) {

	vector<struct SegmentData> segmentDataList;
	const uint32_t raid0_n = getParameters(setting);

	// calculate size of each strip
	const uint32_t stripSize = Coding::roundTo(objectData.info.objectSize,
			raid0_n) / raid0_n;

	for (uint32_t i = 0; i < raid0_n; i++) {

		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;

		if (i == raid0_n - 1) { // last segment
			segmentData.info.segmentSize = objectData.info.objectSize
					- i * stripSize;

		} else {
			segmentData.info.segmentSize = stripSize;
		}

		// TODO: free
		segmentData.buf = MemoryPool::getInstance().poolMalloc(stripSize);

		char* bufPos = objectData.buf + i * stripSize;

		memcpy(segmentData.buf, bufPos, segmentData.info.segmentSize);

		segmentDataList.push_back(segmentData);
	}

	return segmentDataList;
}

struct ObjectData Raid0Coding::decode(vector<struct SegmentData> &segmentData,
		vector<uint32_t> &requiredSegments, uint32_t objectSize,
		string setting) {

	// for raid 0, requiredSegments is not used as all segments are required to decode

	struct ObjectData objectData;

	// copy objectID from first segment
	objectData.info.objectId = segmentData[0].info.objectId;
	objectData.info.objectSize = objectSize;

	objectData.buf = MemoryPool::getInstance().poolMalloc(objectSize);

	uint64_t offset = 0;
	for (struct SegmentData segment : segmentData) {
		memcpy(objectData.buf + offset, segment.buf, segment.info.segmentSize);
		offset += segment.info.segmentSize;
	}

	return objectData;
}

uint32_t Raid0Coding::getParameters(string setting) {
	uint32_t raid0_n;
	istringstream(setting) >> raid0_n;
	return raid0_n;
}

vector<uint32_t> Raid0Coding::getRequiredSegmentIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if any one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	if (failedOsdCount > 0) {
		return {};
	}

	// for Raid0 Coding, require all segments for decode
	const uint32_t raid0_n = getParameters(setting);
	vector<uint32_t> requiredSegments(raid0_n);
	for (uint32_t i = 0; i < raid0_n; i++) {
		requiredSegments[i] = i;
	}
	return requiredSegments;
}

vector<uint32_t> Raid0Coding::getRepairSrcSegmentIds(string setting,
		vector<uint32_t> failedSegments, vector<bool> segmentStatus) {

	debug_error("%s\n", "Repair not supported in RAID0");

	return {};
}

vector<struct SegmentData> Raid0Coding::repairSegments(
		vector<uint32_t> failedSegments,
		vector<struct SegmentData> &repairSrcSegments,
		vector<uint32_t> &repairSrcSegmentId, uint32_t objectSize,
		string setting) {

	debug_error("%s\n", "Repair not supported in RAID0");

	return {};
}
