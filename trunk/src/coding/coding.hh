#ifndef __CODING_HH__
#define __CODING_HH__

#include <vector>
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

class Coding {
public:

	Coding();
	virtual ~Coding();

	virtual vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting) = 0;
	virtual struct ObjectData decode(vector<struct SegmentData> &segmentData,
			vector<uint32_t> &requiredSegments, uint32_t objectSize,
			string setting) = 0;

	virtual vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus) = 0;

	virtual vector<uint32_t> getRepairSrcSegmentIds(string setting,
			vector<uint32_t> failedSegments, vector<bool> segmentStatus) = 0;

	/*
	 the vector size of repairSrcSegments should be the total number of segments for that objects
	 that means some nodes in repairSrcSegments is NULL
	 this design is more convenient as we can address the element by its segmentId
	 */

	virtual vector<struct SegmentData> repairSegments(
			vector<uint32_t> failedSegments,
			vector<struct SegmentData> &repairSrcSegments,
			vector<uint32_t> &repairSrcSegmentId, uint32_t objectSize,
			string setting) = 0;

	uint32_t roundTo(uint32_t numToRound, uint32_t multiple);
	static void bitwiseXor(char* result, char* srcA, char* srcB,
			uint32_t length);

	// For using Memory Pool in Jerasure implementations

	template<class T, class V>
	T* talloc(V num) {
		return (T*) MemoryPool::getInstance().poolMalloc(
				(uint32_t) (sizeof(T) * (num)));
	}

	void tfree(void* ptr) {
		MemoryPool::getInstance().poolFree((char*) ptr);
	}
};

#endif
