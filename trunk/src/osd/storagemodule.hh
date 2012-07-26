#ifndef __STORAGEMODULE_HH__
#define __STORAGEMODULE_HH__

#include <string>
#include <stdint.h>
#include "segmentdata.hh"
#include "objectdata.hh"
using namespace std;

class StorageModule {
public:

	/**
	 * Check if the object exists in the storage
	 * @param objectId Object ID
	 * @return true if object exists, false otherwise
	 */

	bool isObjectExist(uint64_t objectId);

	/**
	 * Read an object from the storage
	 * @param objectId ObjectID
	 * @return ObjectData structure
	 */

	struct ObjectData readObject(uint64_t objectId);

	/**
	 * Read a part of an object from the storage
	 * @param objectId ObjectID
	 * @param offsetInObject Number of bytes to skip
	 * @param length Number of bytes to read
	 * @return ObjectData structure
	 */

	struct ObjectData readObjectTrunk(uint64_t objectId,
			uint64_t offsetInObject, uint32_t length);

	/**
	 * Read a segment from the storage
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @return SegmentData structure
	 */

	struct SegmentData readSegment(uint64_t objectId, uint32_t segmentId);

	/**
	 * Read a part of a segment from the storage
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param offsetInSegment Number of bytes to skip
	 * @param length Number of bytes to read
	 * @return SegmentData structure
	 */

	struct SegmentData readSegmentTrunk(uint64_t objectId, uint32_t segmentId,
			uint64_t offsetInSegment, uint32_t length);

	/**
	 * Write a partial object to the storage
	 * @param objectId Object ID
	 * @param buf Pointer to buffer containing data
	 * @param offsetInObject Offset of the trunk in the object
	 * @param length Number of bytes of the trunk
	 * @return Number of bytes written
	 */

	uint32_t writeObjectTrunk(uint64_t objectId, char* buf,
			uint64_t offsetInObject, uint32_t length);

	/**
	 * Write a partial Segment ID to the storage
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param buf Pointer to buffer containing data
	 * @param offsetInObject Offset of the trunk in the segment
	 * @param length Number of bytes of the trunk
	 * @return Number of bytes written
	 */

	uint32_t writeSegmentTrunk(uint64_t objectId, uint32_t segmentId, char* buf,
			uint64_t offsetInSegment, uint32_t length);

	/**
	 * Create and prepare an object in the storage before writing
	 * @param objectId Object ID
	 * @param length Number of bytes the object will take
	 */

	void createObject(uint64_t objectId, uint32_t length);

	/**
	 * Create an prepare a segment in the storage before writing
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Number of bytes the segment will take
	 */

	void createSegment(uint64_t objectId, uint32_t segmentId, uint32_t length);

	/**
	 * Close the object after the transfer is finished
	 * @param objectId Object ID
	 */

	void closeObject (uint64_t objectId);

	/**
	 * Close the segment after the transfer is finished
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void closeSegment (uint64_t objectId, uint32_t segmentId);

	// getters
	uint32_t getCapacity();
	uint32_t getFreespace();

private:

	/**
	 * Write the information about an object to the database
	 * @param objectId Object ID
	 * @param objectSize Number of bytes the object takes
	 * @param filepath Location of the object in the filesystem
	 */

	void writeObjectInfo(uint64_t objectId, uint32_t objectSize,
			string filepath);

	/**
	 * Read the information about an object from the database
	 * @param objectId Object ID
	 * @return ObjectInfo structure
	 */

	ObjectInfo readObjectInfo(uint64_t objectId);

	/**
	 * Write the information about a segment to the database
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param segmentSize Number of bytes the segment takes
	 * @param filepath Location of the segment in the filesystem
	 */

	void writeSegmentInfo(uint64_t objectId, uint32_t segmentId,
			uint32_t segmentSize, string filepath);

	/**
	 * Read the information about a segment form the database
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @return SegmentInfo structure
	 */

	struct SegmentInfo readSegmentInfo(uint64_t objectId, uint32_t segmentId);

	uint32_t _capacity; 	// total capacity of the node
	uint32_t _freespace;	// remaining capacity of the node
};

#endif
