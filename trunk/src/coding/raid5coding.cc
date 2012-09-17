#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "raid5coding.hh"
#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"

using namespace std;

Raid5Coding::Raid5Coding() {

}

Raid5Coding::~Raid5Coding() {

}

vector<struct SegmentData> Raid5Coding::encode(struct ObjectData objectData,
		string setting) {

	vector<struct SegmentData> segmentDataList;
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t parityIndex = raid5_n - 1; // index starts from 0, last segment is parity
	const uint32_t numDataSegment = raid5_n - 1;
	const uint32_t lastDataIndex = raid5_n - 2;

	if (raid5_n < 3) {
		cerr << "At least 3 segments are needed for RAID-5 encode" << endl;
		exit(-1);
	}

	// calculate size of each data stripe
	const uint32_t stripeSize = Coding::roundTo(objectData.info.objectSize,
			numDataSegment) / numDataSegment;

	// prepare parity segment
	struct SegmentData paritySegmentData;
	paritySegmentData.info.objectId = objectData.info.objectId;
	paritySegmentData.info.segmentId = parityIndex;
	paritySegmentData.info.segmentSize = stripeSize;
	paritySegmentData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);

	// for each data segment
	for (uint32_t i = 0; i < parityIndex; i++) {

		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;

		if (i == lastDataIndex) { // last data segment
			segmentData.info.segmentSize = objectData.info.objectSize
					- i * stripeSize;
		} else {
			segmentData.info.segmentSize = stripeSize;
		}

		// copy data to segment
		segmentData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);
		char* bufPos = objectData.buf + i * stripeSize;
		memcpy(segmentData.buf, bufPos, segmentData.info.segmentSize);

		segmentDataList.push_back(segmentData);

		// update parity segment
		if (i == 0) {
			// first segment: just copy
			memcpy(paritySegmentData.buf, segmentData.buf, stripeSize);
		} else {
			// for second segment onwards, do XOR
			Coding::bitwiseXor(paritySegmentData.buf, paritySegmentData.buf,
					segmentData.buf, stripeSize);
		}
	}

	// add parity segment at the back
	segmentDataList.push_back(paritySegmentData);

	return segmentDataList;
}

struct ObjectData Raid5Coding::decode(vector<struct SegmentData> &segmentData,
		vector<uint32_t> &requiredSegments, uint32_t objectSize,
		string setting) {

	if (requiredSegments.size() < 2) {
		cerr << "At least 2 segments are needed for RAID-5 decode" << endl;
		exit(-1);
	}

	struct ObjectData objectData;
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t parityIndex = raid5_n - 1; // index starts from 0, last segment is parity
	const uint32_t numDataSegment = raid5_n - 1;
	const uint32_t lastDataIndex = raid5_n - 2;

	const uint32_t stripeSize = Coding::roundTo(objectSize,
			numDataSegment) / numDataSegment;

	// copy objectID from first available segment
	objectData.info.objectId = segmentData[requiredSegments[0]].info.objectId;
	objectData.info.objectSize = objectSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(objectSize);

	struct SegmentData rebuildSegmentData;

	// if last segment of requiredSegments is the parity segment, rebuild is needed
	if (requiredSegments.back() == parityIndex) {
		rebuildSegmentData.buf = MemoryPool::getInstance().poolMalloc(
				stripeSize);

		// rebuild
		uint32_t i = 0;
		for (uint32_t segmentId : requiredSegments) {
			if (i == 0) {
				// memcpy first segment
				memcpy(rebuildSegmentData.buf, segmentData[segmentId].buf,
						stripeSize);
			} else {
				// XOR second segment onwards
				Coding::bitwiseXor(rebuildSegmentData.buf,
						rebuildSegmentData.buf, segmentData[segmentId].buf,
						stripeSize);
			}
			i++;
		}

		// write rebuildSegmentData to SegmentData
		uint32_t repairedSegmentIndex = 0;
		set<uint32_t> requiredSegmentsSet(requiredSegments.begin(),
				requiredSegments.end()); // convert to set for efficiency

		// find which segment to recover
		for (uint32_t i = 0; i < segmentData.size(); i++) {
			bool isPresent = requiredSegmentsSet.count(i);

			if (!isPresent) {
				repairedSegmentIndex = i;

				// fill in rebuildSegmentData Information
				rebuildSegmentData.info.segmentId = repairedSegmentIndex;

				if (i == segmentData.size() - 1) {
					rebuildSegmentData.info.segmentSize = objectSize
							- stripeSize * lastDataIndex;
				} else {
					rebuildSegmentData.info.segmentSize = stripeSize;
				}

				// segment does not exist
				segmentData[i] = rebuildSegmentData;

				break; // for raid 5, at most one repair
			}
		}

		// free parity segment
		MemoryPool::getInstance().poolFree(
				segmentData[requiredSegments.back()].buf);

		// replace parity in requiredSegments with repaired segment and sort
		requiredSegments.back() = repairedSegmentIndex;
		sort(requiredSegments.begin(), requiredSegments.end());
	}

	// write segment to object
	uint64_t offset = 0;
	for (uint32_t segmentId : requiredSegments) {
		memcpy(objectData.buf + offset, segmentData[segmentId].buf,
				segmentData[segmentId].info.segmentSize);
		offset += segmentData[segmentId].info.segmentSize;
	}

	return objectData;
}

vector<uint32_t> Raid5Coding::getRequiredSegmentIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	if (failedOsdCount > 1) {
		return {};
	}

	// for raid 5, only requires n-1 stripes (raid5_n - 1) to decode
	const uint32_t raid5_n = getParameters(setting);
	const uint32_t noOfDataSegment = raid5_n - 1;
	vector<uint32_t> requiredSegments;
	requiredSegments.reserve(noOfDataSegment);

	// no OSD failure / parity OSD failure
	if (failedOsdCount == 0
			|| (failedOsdCount == 1 && secondaryOsdStatus.back() == false)) {
		// select first n-1 segments
		for (uint32_t i = 0; i < noOfDataSegment; i++) {
			requiredSegments.push_back(i);
		}
	} else {
		for (uint32_t i = 0; i < raid5_n; i++) {
			// select only available segments
			if (secondaryOsdStatus[i] != false) {
				requiredSegments.push_back(i);
			}
		}
	}

	return requiredSegments;
}

//
// PRIVATE FUNCTION
//

uint32_t Raid5Coding::getParameters(string setting) {
	uint32_t raid5_n;
	istringstream(setting) >> raid5_n;
	return raid5_n;
}
