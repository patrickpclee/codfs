#ifndef __CLIENTSTORAGEMODULE_HH__
#define __CLIENTSTORAGEMODULE_HH__

#include <stdint.h>
#include <vector>
#include <fstream>
#include "../common/segmentdata.hh"
#include "../osd/storagemodule.hh"

using namespace std;

class ClientStorageModule {
public:

	/**
	 * Constructor
	 */
	ClientStorageModule();

	/**
	 * Destructor
	 */
	~ClientStorageModule();

	/**
	 * get the Size of a file
	 * @param filepath 	Path of the file on storage
	 * @return Size of the file
	 */
	uint64_t getFilesize (string filepath);

	/**
	 * get the number of segments of a file
	 * @param filepath 	Path of the file on storage
	 * @return Number of segments
	 */
	uint32_t getSegmentCount (string filepath);

	/**
	 * get the particular segment of a file when given an index.
	 * @param filepath 		Path of the file on storage
	 * @param segmentIndex 	the index of the segment
	 * @return the SegmentData of the segment
	 */
	struct SegmentData readSegmentFromFile (string filepath, uint32_t segmentIndex);

	/**
	 * get the Size of an segment
	 *
	 * @return Size of one segment
	 */
	uint64_t getSegmentSize ();

	/**
	 * Create cache of a segment
	 * @param segmentId 	segment ID
	 * @param segLength 	length of the segment
	 * @param bufLength 	length of the buffer sent
	 */
    void createSegmentCache(uint64_t segmentId, uint32_t segLength, uint32_t bufLength);

	/**
	 * write cache of a segment
	 * @param segmentId 			segment ID
	 * @param buf				buffer contains the data
	 * @param offsetInSegment	offset of the Segment
	 * @param length 			length of the data
	 */
	uint32_t writeSegmentCache (uint64_t segmentId, char* buf, uint64_t offsetInSegment, uint32_t length);

	/**
	 * test whether cache of an segment exist
	 * @param segmentId 			segment ID
	 *
	 * @return true/false
	 */
	bool locateSegmentCache(uint64_t segmentId);

	/**
	 * Close and remove the cache of an segment
	 * @param segmentId segment ID
	 */
	void closeSegment(uint64_t segmentId);

	/**
	 * get the Segment Cache
	 * @param segmentId segment ID
	 *
	 * @return struct SegmentCache contains the segment info.
	 */
	struct SegmentData getSegmentCache(uint64_t segmentId);


	/**
	 * Create a file on disk and open it
	 * @param filepath 	Path of the file on storage
	 * @return Pointer to the opened file
	 */
	FILE* createAndOpenFile(string filepath);

	/**
	 * Open a file and write data from buffer
	 * @param file		file pointer
	 * @param filepath 	Path of the file in the storage
	 * @param buf 		Pointer to source buffer
	 * @param offset 	Offset in the file
	 * @param length 	Length to write
	 * @return Number of bytes written
	 */
	uint32_t writeFile(FILE* file, string filepath, char* buf, uint64_t offset, uint32_t length);

	/**
	 * Close the file and remove it from _openedFile map
	 * @param filePtr 	file Pointer to the file on disk
	 */
	void closeFile (FILE* filePtr);

	void setSegmentCache (uint64_t segmentId, SegmentData segmentCache);

private:
	uint64_t _segmentSize;
	map <uint64_t, struct SegmentData> _segmentCache;
	string _segmentFolder;
};

#endif
