/*
 * storagemodule.cc
 */

#include "storagemodule.hh"

bool StorageModule::isObjectExist(uint64_t objectId) {
	return true;
}

struct ObjectData StorageModule::readObject(uint64_t objectId) {
	struct ObjectData objectData;
	return objectData;
}

struct ObjectData StorageModule::readObjectTrunk(uint64_t objectId,
		uint64_t offsetInObject, uint32_t length) {
	struct ObjectData objectData;
	return objectData;
}

struct SegmentData StorageModule::readSegment(uint64_t objectId,
		uint32_t segmentId) {
	struct SegmentData segmentData;
	return segmentData;
}

struct SegmentData StorageModule::readSegmentTrunk(uint64_t objectId,
		uint32_t segmentId, uint64_t offsetInSegment, uint32_t length) {
	struct SegmentData segmentData;
	return segmentData;
}

uint32_t StorageModule::writeObjectTrunk(uint64_t objectId, char* buf,
		uint64_t offsetInObject, uint32_t length) {
	return 0;
}

uint32_t StorageModule::writeSegmentTrunk(uint64_t objectId, uint32_t segmentId,
		char* buf, uint64_t offsetInSegment, uint32_t length) {
	return 0;
}

void StorageModule::createObject(uint64_t objectId, uint32_t length) {

}

void StorageModule::createSegment(uint64_t objectId, uint32_t segmentId,
		uint32_t length) {

}

uint32_t StorageModule::getCapacity() {
	return 0;
}

uint32_t StorageModule::getFreespace() {
	return 0;
}
