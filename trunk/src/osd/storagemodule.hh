#ifndef __STORAGEMODULE_HH__
#define __STORAGEMODULE_HH__

#include <string>
#include <stdint.h>
#include "segmentdata.hh"
#include "objectdata.hh"
using namespace std;

class StorageModule {
public:

	bool isObjectExist(uint64_t objectId);
	uint32_t writeObject(uint64_t objectId, char* buf);
	uint32_t readObject(uint64_t objectId, char* buf);
	uint32_t readSegment(uint64_t objectId, uint32_t segmentId, char* buf);
	uint32_t writeSegment(uint64_t objectId, uint32_t segmentId, char* buf);
	uint32_t getCapacity();
	uint32_t getFreespace();
private:
	void writeObjectInfo(uint64_t objectId, uint32_t objectSize,
			string filepath);
	ObjectInfo readObjectInfo(uint64_t objectId);
	void writeSegmentInfo(uint64_t objectId, uint32_t segmentId,
			uint32_t segmentSize, string filepath);
	struct SegmentInfo readSegmentInfo(uint64_t objectId, uint32_t segmentId);

	uint32_t _capacity;
	uint32_t _freespace;
};

#endif
