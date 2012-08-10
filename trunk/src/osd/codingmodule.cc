/*
 * codingmodule.cc
 */

#include "codingmodule.hh"
#include "../coding/raid0coding.hh"

CodingModule::CodingModule() {

	_coding = new Raid0Coding();
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		struct ObjectData objectData) {

	return _coding->performEncoding(objectData);
}

struct ObjectData CodingModule::decodeSegmentToObject(uint64_t objectId,
		vector<struct SegmentData> segmentData) {

	return _coding->performDecoding(segmentData);
}

