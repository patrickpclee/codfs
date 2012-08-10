/*
 * codingmodule.cc
 */

#include "codingmodule.hh"
#include "../coding/raid0coding.hh"

CodingModule::CodingModule() {
	const uint32_t noOfStrips = 2;
	_coding = new Raid0Coding(noOfStrips);
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		struct ObjectData objectData) {

	return _coding->performEncoding(objectData);
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(uint64_t objectId,
		char* buf, uint64_t length) {
	struct ObjectData objectData;
	objectData.buf = buf;
	objectData.info.objectId = objectId;
	objectData.info.objectSize = length;
	return _coding->performEncoding(objectData);
}

struct ObjectData CodingModule::decodeSegmentToObject(uint64_t objectId,
		vector<struct SegmentData> segmentData) {

	return _coding->performDecoding(segmentData);
}

