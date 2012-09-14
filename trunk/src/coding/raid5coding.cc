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
	const uint32_t noOfDataStripes = getNoOfDataStripes(setting);
	const uint32_t parityStripeIndex = noOfDataStripes; // data = [0] to [noOfDataStripes -1]

	if (noOfDataStripes < 2) {
		cerr << "At least 2 data stripes are needed for RAID-5 encode" << endl;
		exit(-1);
	}

	// calculate size of each strip
	const uint32_t stripeSize = Coding::roundTo(objectData.info.objectSize,
			noOfDataStripes) / noOfDataStripes;

	// prepare parity segment
	struct SegmentData paritySegmentData;
	paritySegmentData.info.objectId = objectData.info.objectId;
	paritySegmentData.info.segmentId = parityStripeIndex;
	paritySegmentData.info.segmentSize = stripeSize;
	paritySegmentData.buf = MemoryPool::getInstance().poolMalloc(stripeSize);

	for (uint32_t i = 0; i < noOfDataStripes; i++) {

		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;

		if (i == noOfDataStripes - 1) { // last segment
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

	for (uint32_t segmentId : requiredSegments) {
		debug ("before decode, Require ID = %" PRIu32 ", Buf = %p\n", segmentId, segmentData[segmentId].buf);
	}

	if (requiredSegments.size() < 2) {
		cerr << "At least 2 stripes are needed for RAID-5 decode" << endl;
		exit(-1);
	}

	struct ObjectData objectData;
	const uint32_t noOfDataStripes = getNoOfDataStripes(setting);
	const uint32_t parityStripeIndex = noOfDataStripes; // data = [0] to [noOfDataStripes -1]
	const uint32_t stripeSize = roundTo(objectSize, noOfDataStripes)
			/ noOfDataStripes;

	// copy objectID from first available segment
	objectData.info.objectId = segmentData[requiredSegments[0]].info.objectId;
	objectData.info.objectSize = objectSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(objectSize);

	struct SegmentData rebuildSegmentData;

	// if last segment of requiredSegments is the parity segment, rebuild is needed
	if (requiredSegments.back() == parityStripeIndex) {
		rebuildSegmentData.buf = MemoryPool::getInstance().poolMalloc(
				stripeSize);

		debug ("%s\n", "Rebuilding Segment");

		// rebuild
		uint32_t i = 0;
		for (uint32_t segmentId : requiredSegments) {
			if (i == 0) {
				// memcpy first segment
				memcpy(rebuildSegmentData.buf, segmentData[segmentId].buf,
						stripeSize);
		debug ("%s\n", "1) Memcpy");
			} else {
				// XOR second segment onwards
				Coding::bitwiseXor(rebuildSegmentData.buf,
						rebuildSegmentData.buf, segmentData[segmentId].buf,
						stripeSize);
		debug ("%s\n", "2) XOR");
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
				rebuildSegmentData.info.segmentPath = ""; // not used

				if (i == segmentData.size() - 1) {
					rebuildSegmentData.info.segmentSize = objectSize
							- stripeSize * (noOfDataStripes - 1);
				} else {
					rebuildSegmentData.info.segmentSize = stripeSize;
				}

				// segment does not exist
				segmentData[i] = rebuildSegmentData;

				break; // for raid 5, at most one repair
			}
		}

		debug ("Rebuild Segment ID %" PRIu32 " completed\n", repairedSegmentIndex);

		// free parity segment
//		MemoryPool::getInstance().poolFree(segmentData[requiredSegments.back()].buf);

		debug ("Free Parity Segment ID %" PRIu32 " completed\n", requiredSegments.back());

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

	debug ("decode offset = %" PRIu64 "\n", offset);

	for (uint32_t segmentId : requiredSegments) {
		debug ("after decode, Require ID = %" PRIu32 ", Buf = %p\n", segmentId, segmentData[segmentId].buf);
	}


	return objectData;
}

vector<uint32_t> Raid5Coding::getRequiredSegmentIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	debug ("Failed OSD = %" PRIu32 "\n", failedOsdCount);

	if (failedOsdCount > 1) {
		return {};
	}

	// for raid 5, only requires n-1 stripes (noOfDataStripes) to decode
	const uint32_t noOfDataStripes = getNoOfDataStripes(setting);
	vector<uint32_t> requiredSegments;
	requiredSegments.reserve(noOfDataStripes);

	// no OSD failure / parity OSD failure --> select first n-1 stripes
	if (failedOsdCount == 0
			|| (failedOsdCount == 1 && secondaryOsdStatus.back() == false)) {
		for (uint32_t i = 0; i < noOfDataStripes; i++) {
				requiredSegments.push_back(i);
		}
	}

	// one OSD failure
	if (failedOsdCount == 1) {
		for (uint32_t i = 0; i < noOfDataStripes + 1; i++) {
			if (secondaryOsdStatus[i] != false) {
				requiredSegments.push_back(i);
			}
		}
	}

	for (auto segmentId : requiredSegments) {
		debug ("Require Segment ID = %" PRIu32 "\n", segmentId);
	}

	return requiredSegments;
}

//
// PRIVATE FUNCTION
//

uint32_t Raid5Coding::getNoOfDataStripes(string setting) {
	uint32_t noOfDataStripes;
	istringstream(setting) >> noOfDataStripes;
	return noOfDataStripes;
}
