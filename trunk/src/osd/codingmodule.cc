/*
 * codingmodule.cc
 */

#include "codingmodule.hh"
#include "../coding/dummycoding.hh"

CodingModule::CodingModule() {
	// use dummy coding for now
	_coding = new DummyCoding();
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		struct ObjectData objectData) {

	return _coding->performEncoding(objectData);
}

struct ObjectData CodingModule::decodeSegmentToObject(uint64_t objectId,
		vector<struct SegmentData> segmentData) {

	return _coding->performDecoding(segmentData);
}

