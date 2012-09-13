/*
 * codingmodule.cc
 */

#include <thread>
#include <mutex>
#include "codingmodule.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../common/debug.hh"

mutex codingMutex;

CodingModule::CodingModule() {

	{
		lock_guard<mutex> lk(codingMutex);
		_codingWorker[RAID0_CODING] = new Raid0Coding();
		_codingWorker[RAID1_CODING] = new Raid1Coding();
		_codingWorker[RAID5_CODING] = new Raid5Coding();
	}
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		CodingScheme codingScheme, struct ObjectData objectData,
		string setting) {

	return getCoding(codingScheme)->encode(objectData, setting);
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		CodingScheme codingScheme, uint64_t objectId, char* buf,
		uint64_t length, string setting) {

	struct ObjectData objectData;
	objectData.buf = buf;
	objectData.info.objectId = objectId;
	objectData.info.objectSize = length;
	return getCoding(codingScheme)->encode(objectData, setting);
}

struct ObjectData CodingModule::decodeSegmentToObject(CodingScheme codingScheme,
		uint64_t objectId, vector<struct SegmentData> segmentData,
		vector<uint32_t> requiredSegments, uint32_t objectSize,
		string setting) {

	Coding* coding = getCoding(codingScheme);
	struct ObjectData objectData = coding->decode(segmentData, requiredSegments,
			objectSize, setting);

	return objectData;
}

Coding* CodingModule::getCoding(CodingScheme codingScheme) {

	lock_guard<mutex> lk(codingMutex);

	if (!_codingWorker.count(codingScheme)) {
		debug("%s\n", "Wrong coding scheme!");
		exit(-1);
	}

	return _codingWorker[codingScheme];
}

vector<uint32_t> CodingModule::getRequiredSegmentIds(CodingScheme codingScheme,
		string setting, vector<bool> secondaryOsdStatus) {
	return getCoding(codingScheme)->getRequiredSegmentIds(setting,
			secondaryOsdStatus);
}
