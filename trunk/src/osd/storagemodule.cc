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

struct SegmentData StorageModule::readSegment(uint64_t objectId, uint32_t segmentId) {
	struct SegmentData segmentData;
	return segmentData;
}

uint32_t StorageModule::writeObject(struct ObjectData) {
	return 0;
}

uint32_t StorageModule::writeSegment(struct SegmentData) {
	return 0 ;
}

uint32_t StorageModule::getCapacity() {
	return 0;
}

uint32_t StorageModule::getFreespace() {
	return 0;
}
