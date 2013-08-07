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
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../datastructure/concurrentmap.hh"
#include "../common/enums.hh"
#include "filelrucache.hh"

#ifdef USE_IO_THREADS
#include "../../lib/threadpool/threadpool.hpp"
#endif

using namespace std;

/**
 * For retrieving an segment cache file from the disk
 */
struct SegmentDiskCache {
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
     * Check if the segment exists in the storage
     * @param segmentId Segment ID
     * @return true if segment exists, false otherwise
     */

    bool isSegmentCached(uint64_t segmentId);

    /**
     * Creates an SegmentCache for downloading the segment
     * @param segmentId Segment ID
     * @param segLength Length of segment
     * @param bufLength Length of buffer
     */

    void createSegmentCache(uint64_t segmentId, uint32_t segLength,
            uint32_t bufLength, DataMsgType dataMsgType, string updateKey = "");

    /**
     * Create and open the file for storing the block on disk
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param length Length of block
     */

    void createBlock(uint64_t segmentId, uint32_t blockId, uint32_t length);
    void createDeltaBlock (uint64_t segmentId, uint32_t blockId, uint32_t deltaId);
    uint32_t getDeltaCount (uint32_t segmentId, uint32_t blockId);

#ifdef MOUNT_OSD
    /**
     * Create and open the file for storing the block on disk
     * @param osdId OSD ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param length Length of block
     */

    void createRemoteBlock(uint32_t osdId, uint64_t segmentId, uint32_t blockId,
            uint32_t length);
#endif

    /**
     * Read a part of an segment from the storage
     * @param segmentId SegmentID
     * @param offsetInSegment Number of bytes to skip (default 0)
     * @param length Number of bytes to read (read whole segment if 0)
     * @return SegmentData structure
     */

    struct SegmentData readSegment(uint64_t segmentId,
            uint64_t offsetInSegment = 0, uint32_t length = 0);

    /**
     * Read a part of a block from the storage
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param offsetInBlock Number of bytes to skip (default 0)
     * @param length Number of bytes to read (read whole block if 0)
     * @return BlockData structure
     */

    /* deprecated
     struct BlockData readBlock(uint64_t segmentId, uint32_t blockId,
     uint64_t offsetInBlock = 0, uint32_t length = 0);
     */

    /**
     * Read symbols from a block
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param symbols A list of <offset, length> tuples
     * @return BlockData structure
     */

    struct BlockData readBlock(uint64_t segmentId, uint32_t blockId,
            vector<offset_length_t> symbols);
    struct BlockData readDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId);
    void mergeBlock (uint64_t segmentId, uint32_t blockId, bool isParity);
    BlockData getMergedBlock (uint64_t segmentId, uint32_t blockId, bool isParity);

    /**
     * Read symbols from a remote block
     * @param osdId OSD ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param symbols A list of <offset, length> tuples
     * @return BlockData structure
     */

    struct BlockData readRemoteBlock(uint32_t osdId, uint64_t segmentId,
            uint32_t blockId, vector<offset_length_t> symbols);

    /**
     * Write a partial segment to the storage
     * @param segmentId Segment ID
     * @param buf Pointer to buffer containing data
     * @param offsetInSegment Offset of the trunk in the segment
     * @param length Number of bytes of the trunk
     * @return Number of bytes written
     */

    uint32_t writeSegmentDiskCache(uint64_t segmentId, char* buf,
            uint64_t offsetInSegment, uint32_t length);

    /**
     * Write a buffer to the SegmentCache of the segment
     * @param segmentId Segment ID
     * @param buf Pointer to buffer
     * @param offsetInSegment No of bytes to skip in the cache
     * @param length No of bytes to write to the cache
     * @return No of bytes written
     */

    uint32_t writeSegmentData(uint64_t segmentId, char* buf,
            uint64_t offsetInSegment, uint32_t length, DataMsgType dataMsgType,
            string updateKey);

    /**
     * Write a partial Block ID to the storage
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param buf Pointer to buffer containing data
     * @param offsetInSegment Offset of the trunk in the block
     * @param length Number of bytes of the trunk
     * @return Number of bytes written
     */

    uint32_t writeBlock(uint64_t segmentId, uint32_t blockId, char* buf,
            uint64_t offsetInBlock, uint32_t length);
    uint32_t writeDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId, char* buf,
            vector<offset_length_t> offsetLength);

    /**
     * Modify an existing block
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockData BlockData struct
     * @return Number of bytes written
     */

    uint32_t updateBlock(uint64_t segmentId, uint32_t blockId,
            BlockData blockData);

#ifdef MOUNT_OSD
    /**
     * Write a partial Block ID to a remote storage
     * @param osdId Destination OSD ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param buf Pointer to buffer containing data
     * @param offsetInSegment Offset of the trunk in the block
     * @param length Number of bytes of the trunk
     * @return Number of bytes written
     */

    uint32_t writeRemoteBlock(uint32_t osdId, uint64_t segmentId,
            uint32_t blockId, char* buf, uint64_t offsetInBlock,
            uint32_t length);
#endif

    /**
     * Close and remove the segment cache after the transfer is finished
     * @param segmentId Segment ID
     */

    void closeSegmentData(uint64_t segmentId, DataMsgType dataMsgType,
            string updateKey);

    void doFlushFile(string filepath, bool &isFinished);
    void flushFile(string filepath);

    /**
     * Close the segment file
     * @param segmentId Segment ID
     */

    void flushSegmentDiskCache(uint64_t segmentId);

    /**
     * Close the block after the transfer is finished
     * @param segmentId Segment ID
     * @param blockId Block ID
     */

    void flushBlock(uint64_t segmentId, uint32_t blockId);
    void flushDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId);

#ifdef MOUNT_OSD
    /**
     * Close the block after the transfer is finished
     * @param osdId OSD ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     */

    void flushRemoteBlock(uint32_t osdId, uint64_t segmentId, uint32_t blockId);
#endif

    /**
     * Get back the SegmentCache from segmentId
     * @param segmentId 				Segment ID
     *
     * @return SegmentData  Segment Cache
     */

    struct SegmentData getSegmentData(uint64_t segmentId,
            DataMsgType dataMsgType, string updateKey = "");

    /**
     * Set the Capacity of OSD
     * @param max_block capacity of OSD
     */

    void setMaxBlockCapacity(uint32_t max_block);

    /**
     * Set the segment cache space of OSD
     * @param max_segment capacity of segment cache
     */

    void setMaxSegmentCache(uint32_t max_segment);

    /**
     * Get the Capacity of OSD
     * @return uint32_t Max capacity of OSD
     */

    uint32_t getMaxBlockCapacity();

    /**
     * Get the Space of Segment Cache
     * @return uint32_t Max space of segment cache
     */

    uint32_t getMaxSegmentCache();

    /**
     * Get the current Capacity of OSD
     * @return uint32_t current capacity of OSD
     */

    uint32_t getCurrentBlockCapacity();

    /**
     * Get the current usage of segment cache
     * @return uint32_t current usage of segment cache
     */

    uint32_t getCurrentSegmentCache();

    /**
     * Get the free space of OSD
     * @return uint32_t current free space of OSD
     */

    uint32_t getFreeBlockSpace();

    /**
     * Get the free space of segment cache
     * @return uint32_t current free space of segment cache
     */

    uint32_t getFreeSegmentSpace();

    /**
     * Verify whether OSD has enough space
     * @param size 		the required space size
     *
     * @return boolean 	TRUE/FALSE
     */

    bool isEnoughBlockSpace(uint32_t size);

    /**
     * Verify whether segment cache has enough space
     * @param size 		the required space size
     *
     * @return boolean 	TRUE/FALSE
     */

    bool isEnoughSegmentSpace(uint32_t size);

    /**
     * Save segment to segment cache on the disk
     * @param segmentId 		segment ID
     * @param segmentCache 	the segment cache to be saved
     */

    void putSegmentToDiskCache(uint64_t segmentId,
            SegmentData segmentCache);

    /**
     * Read segment cache from the disk
     * @param segmentId 		segment ID
     *
     * @return segmentData 	the segment data from the cache
     */

    struct SegmentData getSegmentFromDiskCache(uint64_t segmentId);

    /**
     * Clear all segment disk cache
     */

    void clearSegmentDiskCache();

    /**
     * Return the list of cached segment ID
     * @return ID of cached segments
     */

    list<uint64_t> getSegmentCacheQueue();

    static uint32_t getCombinedLength(vector<offset_length_t> offsetLength);

private:

    /**
     * Calculate and update the free space and usage while set up
     */
    void initializeStorageStatus();

    /**
     * Calculate and update the free space and usage of OSD
     * @param size the size of the block to be saved
     */
    void updateBlockFreespace(uint32_t size);

    /**
     * Calculate and update the free space and usage of segment cache
     * @param new_segment_size	the size of the segment cache to be saved
     */
    void updateSegmentFreespace(uint32_t new_segment_size);

    /**
     * Delete old entry in the segment cache to make room for new one
     * @param new_segment_size	the size of the segment cache to be saved
     *
     * @return int32_t 			the updated size of segment cache after deletion
     */
    int32_t spareSegmentSpace(uint32_t new_segment_size);

    /**
     * Write the information about an segment to the database
     * @param segmentId Segment ID
     * @param segmentSize Number of bytes the segment takes
     * @param filepath Location of the segment in the filesystem

     void writeSegmentInfo(uint64_t segmentId, uint32_t segmentSize,
     string filepath);
     */

    /**
     * Read the information about an segment from the database
     * @param segmentId Segment ID
     * @return SegmentInfo structure
     */

    struct SegmentInfo readSegmentInfo(uint64_t segmentId);

    /**
     * Write the information about a block to the database
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockSize Number of bytes the block takes
     * @param filepath Location of the block in the filesystem
     */

    void writeBlockInfo(uint64_t segmentId, uint32_t blockId,
            uint32_t blockSize, string filepath);

    /**
     * Read the information about a block form the database
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @return BlockInfo structure
     */

//	struct BlockInfo readBlockInfo(uint64_t segmentId, uint32_t blockId);
    /**
     * Open a file and read data to buffer
     * @param filepath Path of the file in the storage
     * @param buf Pointer to destination buffer (already malloc-ed)
     * @param offset Offset in the file
     * @param length Length to read
     * @return Number of bytes read
     */

    uint32_t readFile(string filepath, char* buf, uint64_t offset,
            uint32_t length, bool isCache, int priority = 10);

    uint32_t doReadFile(string filepath, char* buf, uint64_t offset,
            uint32_t length, bool isCache, bool &isFinished);

    /**
     * Open a file and write data from buffer
     * @param filepath Path of the file in the storage
     * @param buf Pointer to source buffer
     * @param offset Offset in the file
     * @param length Length to write
     * @return Number of bytes written
     */

    uint32_t writeFile(string filepath, char* buf, uint64_t offset,
            uint32_t length, bool isCache, int priority = 10);

    uint32_t doWriteFile(string filepath, char* buf, uint64_t offset,
            uint32_t length, bool isCache, bool &isFinished);

    /**
     * Return the segment path given Segment ID
     * @param segmentId Segment ID
     * @param segmentFolder Location where segments are stored
     * @return filepath of the segment in the filesystem
     */

    string generateSegmentPath(uint64_t segmentId, string segmentFolder);

    /**
     * Return the block path given Segment ID and Block ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockFolder Location where blocks are stored
     * @return filepath of the block in the filesystem
     */

    string generateBlockPath(uint64_t segmentId, uint32_t blockId,
            string blockFolder);
    string generateDeltaBlockPath(uint64_t segmentId, uint32_t blockId, uint32_t deltaId,
            string blockFolder);

#ifdef MOUNT_OSD
    /**
     * Return the block path given Segment ID and Block ID
     * @param osdId Destination OSD ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockFolder Location where blocks are stored
     * @return filepath of the block in the filesystem
     */

    string generateRemoteBlockPath(uint32_t osdId, uint64_t segmentId,
            uint32_t blockId, string blockFolder);
#endif

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
     * Try to close the file before remove
     * @param filepath Path to the file on disk
     */

    void tryCloseFile(string filepath);

    /**
     * Open the file and finds the size of it
     * @param filepath Path to the file on disk
     * @return Size of the file
     */
    uint64_t getFilesize(string filepath);

    // TODO: use more efficient data structure for LRU delete
    ConcurrentMap<uint64_t, struct SegmentDiskCache> _segmentDiskCacheMap;
    list<uint64_t> _segmentDiskCacheQueue;

    FileLruCache<string, FILE*>* _openedFile;
    map<uint64_t, struct SegmentData> _segmentUploadCache;
    map<string, struct SegmentData> _segmentUpdateCache;
    string _segmentFolder;
    string _blockFolder;
    string _remoteBlockFolder;
    uint64_t _maxBlockCapacity;
    uint64_t _maxSegmentCache;
    atomic<uint64_t> _freeBlockSpace;
    atomic<uint64_t> _freeSegmentSpace;
    atomic<uint32_t> _currentBlockUsage;
    atomic<uint32_t> _currentSegmentUsage;

#ifdef USE_IO_THREADS
    boost::threadpool::prio_pool _iotp;
#endif
};

#endif
