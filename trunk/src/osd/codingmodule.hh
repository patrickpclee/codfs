/**
 * codingmodule.hh
 */

#ifndef __CODINGMODULE_HH__
#define __CODINGMODULE_HH__
#include <list>
#include <stdint.h>
#include "segmentdata.hh"
#include "objectdata.hh"

class CodingModule {
public:

	/**
	 * Encode an object to a list of segments
	 * @param objectData
	 * @return A list of SegmentData structure
	 */

	list<struct SegmentData> encodeObjectToSegment(
			struct ObjectData objectData);

	/**
	 * Decode a list of segments into an object
	 * @param objectId Destination object ID
	 * @param segmentData a list of SegmentData structure
	 * @return an ObjectData structure
	 */

	struct ObjectData decodeSegmentToObject(uint64_t objectId,
			list<struct SegmentData> segmentData);

	/**
	 * Retrieve a segment from the storage module
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId ID of the segment to retrieve
	 * @return a SegmentData structure
	 */

};

#endif
