/*
 * codingmodule.cc
 */

#include <thread>
#include "codingmodule.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../common/debug.hh"

mutex codingMutex;

CodingModule::CodingModule() {

	{
		lock_guard<mutex> lk(codingMutex);
		_codingWorker[RAID0_CODING] = new Raid0Coding();
		_codingWorker[RAID1_CODING] = new Raid1Coding();
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
		string setting) {

	Coding* coding = getCoding(codingScheme);
	struct ObjectData objectData = coding->decode(segmentData, setting);

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
		string setting) {
	return getCoding(codingScheme)->getRequiredSegmentIds(setting);
}

/*
 Coding* CodingModule::codingWorkerFactory (CodingScheme codingScheme) {

 Coding* coding = NULL;

 if (codingScheme == RAID0_CODING) {
 const uint32_t noOfStrips = 2;
 coding = new Raid0Coding(noOfStrips);
 } else if (codingScheme == RAID1_CODING) {
 const uint32_t noOfReplications = 3;
 coding = new Raid1Coding(noOfReplications);
 }

 if (coding == NULL) {
 debug ("%s\n", "Wrong Coding Scheme specified");
 exit (-1);
 }

 return coding;
 }
 */
