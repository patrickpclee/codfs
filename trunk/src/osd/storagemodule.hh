#ifndef __STORAGEMODULE_HH__
#define __STORAGEMODULE_HH__

#include <string>
#include <stdint.h>
#include <map>
#include <list>
#include <stdio.h>
#include <atomic>
#include "../config/config.hh"
#include "../common/memorypool.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"
#include "../datastructure/concurrentmap.hh"
#include "../cache/lru_cache.hh"
using namespace std;

/**
 * For caching an object in memory during upload/download
 */

struct ObjectTransferCache {
	uint32_t length;
	char* buf;
};

/**
 * For retrieving an object cache file from the disk
 */
struct ObjectDiskCache {
	uint32_t length;
	string filepath;
	struct timespec lastAccessedTime;
};

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

	bool isObjectCached(uint64_t objectId);

	/**
	 * Creates an ObjectCache for downloading the object
	 * @param objectId Object ID
	 * @param length Length of object
	 */

	void createObjectCache(uint64_t objectId, uint32_t length);

	/**
	 * Create and open the file for storing the object on disk
	 * @param objectId Object ID
	 * @param length Length of object
	 */

	void createObjectFile(uint64_t objectId, uint32_t length);

	/**
	 * Create and open the file for storing the segment on disk
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Length of segment
	 */

	void createSegment(uint64_t objectId, uint32_t segmentId, uint32_t length);

	/**
	 * Read a part of an object from the storage
	 * @param objectId ObjectID
	 * @param offsetInObject Number of bytes to skip (default 0)
	 * @param length Number of bytes to read (read whole object if 0)
	 * @return ObjectData structure
	 */

	struct ObjectData readObject(uint64_t objectId, uint64_t offsetInObject = 0,
			uint32_t length = 0);

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

	uint32_t writeObjectFile(uint64_t objectId, char* buf,
			uint64_t offsetInObject, uint32_t length);

	/**
	 * Write a buffer to the ObjectCache of the object
	 * @param objectId Object ID
	 * @param buf Pointer to buffer
	 * @param offsetInObject No of bytes to skip in the cache
	 * @param length No of bytes to write to the cache
	 * @return No of bytes written
	 */

	uint32_t writeObjectCache(uint64_t objectId, char* buf,
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

	FILE* createAndOpenObjectFile(uint64_t objectId, uint32_t length);

	/**
	 * Create an prepare a segment in the storage before writing
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Number of bytes the segment will take
	 */

	FILE* createAndOpenSegment(uint64_t objectId, uint32_t segmentId,
			uint32_t length);

	/**
	 * Close and remove the object cache after the transfer is finished
	 * @param objectId Object ID
	 */

	void closeObjectCache(uint64_t objectId);

	/**
	 * Close the object file
	 * @param objectId Object ID
	 */

	void closeObjectFile(uint64_t objectId);

	/**
	 * Close the segment after the transfer is finished
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void closeSegment(uint64_t objectId, uint32_t segmentId);

	/**
	 * Get back the ObjectCache from objectId
	 * @param objectId 				Object ID
	 *
	 * @return ObjectTransferCache  Object Cache
	 */

	struct ObjectTransferCache getObjectCache(uint64_t objectId);

	/**
	 * Set the Capacity of OSD
	 * @param max_segment capacity of OSD
	 */

	void setMaxSegmentCapacity(uint32_t max_segment);

	/**
	 * Set the object cache space of OSD
	 * @param max_object capacity of object cache
	 */

	void setMaxObjectCache(uint32_t max_object);

	/**
	 * Get the Capacity of OSD
	 * @return uint32_t Max capacity of OSD
	 */

	uint32_t getMaxSegmentCapacity();

	/**
	 * Get the Space of Object Cache
	 * @return uint32_t Max space of object cache
	 */

	uint32_t getMaxObjectCache();

	/**
	 * Get the current Capacity of OSD
	 * @return uint32_t current capacity of OSD
	 */

	uint32_t getCurrentSegmentCapacity();

	/**
	 * Get the current usage of object cache
	 * @return uint32_t current usage of object cache
	 */

	uint32_t getCurrentObjectCache();

	/**
	 * Get the free space of OSD
	 * @return uint32_t current free space of OSD
	 */

	uint32_t getFreeSegmentSpace();

	/**
	 * Get the free space of object cache
	 * @return uint32_t current free space of object cache
	 */

	uint32_t getFreeObjectSpace();

	/**
	 * Verify whether OSD has enough space
	 * @param size 		the required space size
	 *
	 * @return boolean 	TRUE/FALSE
	 */

	bool verifySegmentSpace(uint32_t size);

	/**
	 * Verify whether object cache has enough space
	 * @param size 		the required space size
	 *
	 * @return boolean 	TRUE/FALSE
	 */

	bool verifyObjectSpace(uint32_t size);

	/**
	 * Save object to object cache on the disk
	 * @param objectId 		object ID
	 * @param objectCache 	the object cache to be saved
	 */

	void saveObjectToDisk(uint64_t objectId, ObjectTransferCache objectCache);

	/**
	 * Read object cache from the disk
	 * @param objectId 		object ID
	 *
	 * @return objectData 	the object data from the cache
	 */

	struct ObjectData getObjectFromDiskCache(uint64_t objectId);

	void clearObjectDiskCache();

private:

	/**
	 * Calculate and update the free space and usage while set up
	 */
	void initializeStorageStatus();

	/**
	 * Calculate and update the free space and usage of OSD
	 * @param new_segment_size	the size of the segment to be saved
	 */
	void updateSegmentFreespace(uint32_t new_segment_size);

	/**
	 * Calculate and update the free space and usage of object cache
	 * @param new_object_size	the size of the object cache to be saved
	 */
	void updateObjectFreespace(uint32_t new_object_size);

	/**
	 * Delete old entry in the object cache to make room for new one
	 * @param new_object_size	the size of the object cache to be saved
	 *
	 * @return int32_t 			the updated size of object cache after deletion
	 */
	int32_t spareObjectSpace(uint32_t new_object_size);

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

//	struct SegmentInfo readSegmentInfo(uint64_t objectId, uint32_t segmentId);
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

	/**
	 * Create a file on disk and open it
	 * @param filepath Path of the file on storage
	 * @return Pointer to the opened file
	 */

	FILE* createFile(string filepath);

	/**
	 * Retrieve the opened file pointer if file is already open
	 * Open the file on disk if file is not already open
	 * @param filepath Path to the file on disk
	 * @return Pointer to the opened file
	 */

	FILE* openFile(string filepath);

	/**
	 * Close the file and remove it from _openedFile map
	 * @param filepath Path to the file on disk
	 */

	void closeFile(string filepath);

	/**
	 * Open the file and finds the size of it
	 * @param filepath Path to the file on disk
	 * @return Size of the file
	 */
	uint64_t getFilesize(string filepath);

	// TODO: use more efficient data structure for LRU delete
	ConcurrentMap<uint64_t, struct ObjectDiskCache> _objectDiskCacheMap;
	list<uint64_t> _objectCacheQueue;

	//LruCache <string, FILE*> _openedFile;
	map<string, FILE*> _openedFile;
	map<uint64_t, struct ObjectTransferCache> _objectCache;
	string _objectFolder;
	string _segmentFolder;
	uint64_t _maxSegmentCapacity;
	uint64_t _maxObjectCache;
	atomic<uint64_t> _freeSegmentSpace;
	atomic<uint64_t> _freeObjectSpace;
	atomic<uint32_t> _currentSegmentUsage;
	atomic<uint32_t> _currentObjectUsage;
};

#endif
