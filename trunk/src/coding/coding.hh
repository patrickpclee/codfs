#ifndef __CODING_HH__
#define __CODING_HH__

#include <vector>
#include "../common/segmentdata.hh"
#include "../common/blockdata.hh"
#include "../common/memorypool.hh"

class Coding {
public:

	Coding();
	virtual ~Coding();

	virtual vector<struct BlockData> encode(struct SegmentData segmentData,
			string setting) = 0;
	virtual struct SegmentData decode(vector<struct BlockData> &blockData,
			vector<uint32_t> &requiredBlocks, uint32_t segmentSize,
			string setting) = 0;

	virtual vector<uint32_t> getRequiredBlockIds(string setting,
			vector<bool> secondaryOsdStatus) = 0;

	virtual vector<uint32_t> getRepairSrcBlockIds(string setting,
			vector<uint32_t> failedBlocks, vector<bool> blockStatus) = 0;

	/*
	 the vector size of repairSrcBlocks should be the total number of blocks for that segments
	 that means some nodes in repairSrcBlocks is NULL
	 this design is more convenient as we can address the element by its blockId
	 */

	virtual vector<struct BlockData> repairBlocks(
			vector<uint32_t> failedBlocks,
			vector<struct BlockData> &repairSrcBlocks,
			vector<uint32_t> &repairSrcBlockId, uint32_t segmentSize,
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
