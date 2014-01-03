#ifndef __CODING_HH__
#define __CODING_HH__

#include <vector>
#include <utility>
#include "../common/segmentdata.hh"
#include "../common/blockdata.hh"
#include "../common/memorypool.hh"
#include "../common/define.hh"

using namespace std;

class Coding {
public:

	/**
	 * Constructor
	 */

	Coding();

	/**
	 * Destructor
	 */

	virtual ~Coding();

	/**
	 * Encode a segment to a list of blocks
	 * @param segmentData SegmentData structure
	 * @param setting Coding setting
	 * @return a list of BlockData structure
	 */

	virtual vector<BlockData> encode(struct SegmentData segmentData,
			string setting) = 0;

	/**
	 * Decode a list of blocks to a segment
	 * @param blockData a list of BlockData structure
	 * @param symbolList tuples returned by getRequiredBlockSymbols
	 * @param segmentSize Original Segment Size
	 * @param setting Coding setting
	 * @return
	 */

	virtual SegmentData decode(vector<BlockData> &blockDataList,
			block_list_t &symbolList, uint32_t segmentSize, string setting) = 0;

	/**
	 * Get the information about symbols required for decode
	 * @param blockStatus True if block[i] is available, false otherwise
	 * @param segmentSize Segment Size
	 * @param setting Coding Setting
	 * @return vector <blockId, vector <offset, length>>
	 */

	virtual block_list_t getRequiredBlockSymbols(vector<bool> blockStatus,
			uint32_t segmentSize, string setting) = 0;

	/**
	 * Get the information about symbols required for repair
	 * @param failedBlocks List of failed blocks
	 * @param blockStatus True if block[i] is available, false otherwise
	 * @param segmentSize Segment Size
	 * @param setting Coding Setting
	 * @return vector <blockId, vector <symbol no> >
	 */

	virtual block_list_t getRepairBlockSymbols(vector<uint32_t> failedBlocks,
			vector<bool> blockStatus, uint32_t segmentSize, string setting) = 0;

	/**
	 * Repair blocks using other blocks
	 * @param repairBlockIdList List of blocks to decode
	 * @param blockData a list of BlockData structure  **contains NULL for not required elements**
	 * @param blockIdList Block ID
	 * @param symbolList tuples returned by getRequiredBlockSymbols
	 * @param segmentSize Original Segment Size
	 * @param setting Coding setting
	 * @return List of decoded BlockData
	 */

	virtual vector<struct BlockData> repairBlocks(
			vector<uint32_t> repairBlockIdList,
			vector<struct BlockData> &blockData, block_list_t &symbolList,
			uint32_t segmentSize, string setting) = 0;

	virtual uint32_t getBlockCountFromSetting (string setting) = 0;

	virtual uint32_t getParityCountFromSetting (string setting);

	virtual uint32_t getBlockSize(uint32_t segmentSize, string setting) = 0; 

	virtual vector<BlockData> computeDelta(BlockData oldBlock, BlockData newBlock,
	        vector<offset_length_t> offsetLength, vector<uint32_t> parityVector);

	/**
	 * Round up a number to the nearest multiple
	 * @param numToRound Number to round
	 * @param multiple Multiple
	 * @return Rounded number
	 */

	uint32_t roundTo(uint32_t numToRound, uint32_t multiple);

	/**
	 * Efficient XOR function
	 * @param result XOR-ed result
	 * @param srcA First number
	 * @param srcB Second number
	 * @param length Number of bytes to do XOR
	 */

	static void bitwiseXor(char* result, char* srcA, char* srcB,
			uint32_t length);

	uint32_t getCombinedLength(vector<offset_length_t> offsetLength);

	// For using Memory Pool in Jerasure implementations

	/**
	 * Allocating memory in Jerasure
	 * @param num Number of element V to allocate
	 * @return Pointer to allocated memory
	 */

	template<class T, class V>
	T* talloc(V num) {
		return (T*) MemoryPool::getInstance().poolMalloc(
				(uint32_t) (sizeof(T) * (num)));
	}

	/**
	 * Free memory in Jerasure
	 * @param ptr Pointer to allocated memory
	 */

	void tfree(void* ptr) {
		MemoryPool::getInstance().poolFree((char*) ptr);
	}
};

#endif
