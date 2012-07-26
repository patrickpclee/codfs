#ifndef __STORAGEMODULE_HH__
#define __STORAGEMODULE_HH__

#include <string>
#include <stdint.h>
#include <map>
#include <stdio.h>
#include "../config/config.hh"
#include "../common/memorypool.hh"
#include "segmentdata.hh"
#include "objectdata.hh"
using namespace std;

class StorageModule {
public:

	/**
	 * Constructor
	 */

	StorageModule();

	/**
	 * Destructor
	 */

	~StorageModule();

	/**
	 * Check if the object exists in the storage
	 * @param objectId Object ID
	 * @return true if object exists, false otherwise
	 */

	bool isObjectExist(uint64_t objectId);

	void createObject (uint64_t objectId, uint32_t length);
	void createSegment (uint64_t objectId, uint32_t segmentId, uint32_t length);

	/**
	 * Read a part of an object from the storage
	 * @param objectId ObjectID
	 * @param offsetInObject Number of bytes to skip (default 0)
	 * @param length Number of bytes to read (read whole object if 0)
	 * @return ObjectData structure
	 */

	struct ObjectData readObject(uint64_t objectId,
			uint64_t offsetInObject = 0, uint32_t length = 0);

	/**
	 * Read a part of a segment from the storage
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param offsetInSegment Number of bytes to skip (default 0)
	 * @param length Number of bytes to read (read whole segment if 0)
	 * @return SegmentData structure
	 */

	struct SegmentData readSegment(uint64_t objectId, uint32_t segmentId,
			uint64_t offsetInSegment = 0, uint32_t length = 0);

	/**
	 * Write a partial object to the storage
	 * @param objectId Object ID
	 * @param buf Pointer to buffer containing data
	 * @param offsetInObject Offset of the trunk in the object
	 * @param length Number of bytes of the trunk
	 * @return Number of bytes written
	 */

	uint32_t writeObject(uint64_t objectId, char* buf,
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

	uint32_t writeSegment(uint64_t objectId, uint32_t segmentId, char* buf,
			uint64_t offsetInSegment, uint32_t length);

	/**
	 * Create and prepare an object in the storage before writing
	 * @param objectId Object ID
	 * @param length Number of bytes the object will take
	 */

	FILE* createAndOpenObject(uint64_t objectId, uint32_t length);

	/**
	 * Create an prepare a segment in the storage before writing
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Number of bytes the segment will take
	 */

	FILE* createAndOpenSegment(uint64_t objectId, uint32_t segmentId, uint32_t length);

	/**
	 * Close the object after the transfer is finished
	 * @param objectId Object ID
	 */

	void closeObject(uint64_t objectId);

	/**
	 * Close the segment after the transfer is finished
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void closeSegment(uint64_t objectId, uint32_t segmentId);

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

	struct ObjectInfo readObjectInfo(uint64_t objectId);

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

	/**
	 * Open a file and read data to buffer
	 * @param filepath Path of the file in the storage
	 * @param buf Pointer to destination buffer (already malloc-ed)
	 * @param offset Offset in the file
	 * @param length Length to read
	 * @return Number of bytes read
	 */

	uint32_t readFile(string filepath, char* buf, uint64_t offset,
			uint32_t length);

	/**
	 * Open a file and write data from buffer
	 * @param filepath Path of the file in the storage
	 * @param buf Pointer to source buffer
	 * @param offset Offset in the file
	 * @param length Length to write
	 * @return Number of bytes written
	 */

	uint32_t writeFile(string filepath, char* buf, uint64_t offset,
			uint32_t length);

	/**
	 * Return the object path given Object ID
	 * @param objectId Object ID
	 * @param objectFolder Location where objects are stored
	 * @return filepath of the object in the filesystem
	 */

	string generateObjectPath(uint64_t objectId, string objectFolder);

	/**
	 * Return the segment path given Object ID and Segment ID
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param segmentFolder Location where segments are stored
	 * @return filepath of the segment in the filesystem
	 */

	string generateSegmentPath(uint64_t objectId, uint32_t segmentId,
			string segmentFolder);

	FILE* createFile (string filepath);

	FILE* openFile (string filepath);

	void closeFile (string filepath);

	uint32_t _capacity; // total capacity of the node
	uint32_t _freespace; // remaining capacity of the node
	map <string, FILE*> _openedFile;
	string _objectFolder;
	string _segmentFolder;
};

#endif
