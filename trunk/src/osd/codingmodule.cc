/*
 * codingmodule.cc
 */

#include "codingmodule.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../common/debug.hh"

CodingModule::CodingModule() {

	const uint32_t noOfStrips = 2;
	_codingWorker[RAID0_CODING] = new Raid0Coding(noOfStrips);

	const uint32_t noOfReplications = 3;
	_codingWorker[RAID1_CODING] = new Raid1Coding(noOfReplications);
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		CodingScheme codingScheme, struct ObjectData objectData) {

	return getCoding(codingScheme)->performEncoding(objectData);
}

vector<struct SegmentData> CodingModule::encodeObjectToSegment(
		CodingScheme codingScheme, uint64_t objectId, char* buf,
		uint64_t length) {
	struct ObjectData objectData;
	objectData.buf = buf;
	objectData.info.objectId = objectId;
	objectData.info.objectSize = length;
	return getCoding(codingScheme)->performEncoding(objectData);
}

struct ObjectData CodingModule::decodeSegmentToObject(CodingScheme codingScheme,
		uint64_t objectId, vector<struct SegmentData> segmentData) {

	return getCoding(codingScheme)->performDecoding(segmentData);
}

Coding* CodingModule::getCoding(CodingScheme codingScheme) {
	if (!_codingWorker.count(codingScheme)) {
		debug("%s\n", "Wrong coding scheme!");
		exit(-1);
	}

	return _codingWorker[codingScheme];
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
