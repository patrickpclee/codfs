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
#include "reservespaceinfo.hh"
#include "deltalocation.hh"

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

typedef boost::shared_mutex RWMutex;
typedef boost::shared_lock<boost::shared_mutex> readLock;
typedef boost::unique_lock<boost::shared_mutex> writeLock;

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
     * Creates an SegmentCache for downloading the segment
     * @param segmentId Segment ID
     * @param segLength Length of segment
     * @param bufLength Length of buffer
     */

    void createSegmentTransferCache(uint64_t segmentId, uint32_t segLength,
            uint32_t bufLength, DataMsgType dataMsgType, string updateKey = "");

    /**
     * Create and open the file for storing the block on disk
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param length Length of block
     */

    void createBlock(uint64_t segmentId, uint32_t blockId, uint32_t length);
    void createDeltaBlock(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId, bool useReserve);
    void reserveBlockSpace(uint64_t segmentId, uint32_t blockId,
            uint32_t offset, uint32_t blockSize, uint32_t reserveLength);
    uint32_t getDeltaCount(uint32_t segmentId, uint32_t blockId);
    uint32_t getNextDeltaId(uint32_t segmentId, uint32_t blockId);

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

    BlockData getBlock(uint64_t segmentId, uint32_t blockId, bool isParity,
            vector<offset_length_t> symbols, bool needLock);

    /**
     * Read symbols from a block
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param symbols A list of <offset, length> tuples
     * @return BlockData structure
     */

    struct BlockData readBlock(uint64_t segmentId, uint32_t blockId,
            vector<offset_length_t> symbols);
    struct BlockData readDeltaBlock(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId);
    void mergeBlock(uint64_t segmentId, uint32_t blockId, bool isParity);
    BlockData getMergedBlock(uint64_t segmentId, uint32_t blockId,
            bool isParity, bool needLock);
    BlockData readDeltaFromReserve(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId, DeltaLocation deltaLocation);
    BlockData doReadDelta(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId, bool isReserve, uint32_t offset);

    /**
     * Write a buffer to the SegmentCache of the segment
     * @param segmentId Segment ID
     * @param buf Pointer to buffer
     * @param offsetInSegment No of bytes to skip in the cache
     * @param length No of bytes to write to the cache
     * @return No of bytes written
     */

    uint32_t writeSegmentTransferCache(uint64_t segmentId, char* buf,
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
    uint32_t writeDeltaBlock(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId, char* buf, vector<offset_length_t> offsetLength,
            bool isParity);

    /**
     * Modify an existing block
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockData BlockData struct
     * @return Number of bytes written
     */

    uint32_t updateBlock(uint64_t segmentId, uint32_t blockId,
            BlockData blockData);

    /**
     * Close and remove the segment cache after the transfer is finished
     * @param segmentId Segment ID
     */

    void closeSegmentTransferCache(uint64_t segmentId, DataMsgType dataMsgType,
            string updateKey);

    void flushFile(string filepath);

    struct SegmentData getSegmentTransferCache(uint64_t segmentId,
            DataMsgType dataMsgType, string updateKey = "");

    /**
     * Close the block after the transfer is finished
     * @param segmentId Segment ID
     * @param blockId Block ID
     */

    void flushBlock(uint64_t segmentId, uint32_t blockId);
    void flushDeltaBlock(uint64_t segmentId, uint32_t blockId, uint32_t deltaId,
            bool isParity);

    /**
     * Set the Capacity of OSD
     * @param max_block capacity of OSD
     */

    void setMaxBlockCapacity(uint32_t max_block);

    /**
     * Get the Capacity of OSD
     * @return uint32_t Max capacity of OSD
     */

    uint64_t getMaxBlockCapacity();

    /**
     * Get the current Capacity of OSD
     * @return uint32_t current capacity of OSD
     */

    uint64_t getCurrentBlockCapacity();

    /**
     * Get the free space of OSD
     * @return uint32_t current free space of OSD
     */

    uint64_t getFreeBlockSpace();

    /**
     * Verify whether OSD has enough space
     * @param size 		the required space size
     *
     * @return boolean 	TRUE/FALSE
     */

    bool isEnoughBlockSpace(uint32_t size);

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
     * Open a file and read data to buffer
     * @param filepath Path of the file in the storage
     * @param buf Pointer to destination buffer (already malloc-ed)
     * @param offset Offset in the file
     * @param length Length to read
     * @return Number of bytes read
     */

    uint32_t readFile(string filepath, char* buf, uint64_t offset,
            uint32_t length, bool isCache, int priority = 10);

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

    /**
     * Return the block path given Segment ID and Block ID
     * @param segmentId Segment ID
     * @param blockId Block ID
     * @param blockFolder Location where blocks are stored
     * @return filepath of the block in the filesystem
     */

    string generateBlockPath(uint64_t segmentId, uint32_t blockId,
            string blockFolder);
    string generateDeltaBlockPath(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId, string blockFolder);

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

    RWMutex* obtainRWMutex(string blockKey);

//    DeltaLocation getDeltaLocation (uint64_t segmentId, uint32_t blockId, uint32_t deltaId);
    string getBlockKey(uint64_t segmentId, uint32_t blockId);
    string getBlockKey(string segmentId, string blockId);
    string generateDeltaKey(uint64_t segmentId, uint32_t blockId,
            uint32_t deltaId);

    FileLruCache<string, FILE*>* _openedFile;
    map<uint64_t, struct SegmentData> _segmentUploadCache;
    map<string, struct SegmentData> _segmentUpdateCache;
    string _blockFolder;
    string _remoteBlockFolder;
    uint64_t _maxBlockCapacity;
    atomic<uint64_t> _freeBlockSpace;
    atomic<uint64_t> _currentBlockUsage;

    ConcurrentMap<string, uint32_t> _deltaIdMap;
    ConcurrentMap<string, vector<offset_length_t>> _deltaOffsetLength;
    ConcurrentMap<string, vector<DeltaLocation>> _deltaLocationMap;
    ConcurrentMap<string, ReserveSpaceInfo> _reserveSpaceMap;

    unordered_map<string, boost::shared_mutex*> _deltaRWMutexMap;
    mutex _deltaRWMutexMapMutex;

    uint32_t _updateScheme;
    uint64_t _reservedSpaceSize;
};

#endif
