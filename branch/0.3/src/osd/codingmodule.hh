/**
 * codingmodule.hh
 */

#ifndef __CODINGMODULE_HH__
#define __CODINGMODULE_HH__
#include <map>
#include <vector>
#include <stdint.h>
#include "../coding/coding.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/enums.hh"

struct CodingSetting {
    CodingScheme codingScheme;
    string setting;
};

class CodingModule {
    public:

        CodingModule();

        /**
         * Encode an segment to a list of blocks
         * @param codingScheme Coding Scheme
         * @param segmentData SegmentData structure
         * @param setting Setting for the coding scheme
         * @return A list of BlockData structure
         */

        vector<BlockData> encodeSegmentToBlock(CodingScheme codingScheme,
                SegmentData segmentData, string setting);

        /**
         * Encode an segment to a list of blocks
         * @param codingScheme Coding Scheme
         * @param segmentId Segment ID
         * @param buf Pointer to buffer holding segment data
         * @param length length of the segment
         * @param setting Setting for the coding scheme
         * @return A list of BlockData structure
         */

        vector<BlockData> encodeSegmentToBlock(CodingScheme codingScheme,
                uint64_t segmentId, char* buf, uint64_t length, string setting);

        /**
         * Decode a list of blocks into an segment
         * @param codingScheme Coding Scheme
         * @param blockDataList A list of BlockData structure that are used for decode
         * @param symbolList blockId and offset length in blockDataList for decode
         * @param segmentSize Size of the original segment
         * @param setting Setting for the coding scheme
         * @return an SegmentData structure
         */

        SegmentData decodeBlockToSegment(CodingScheme codingScheme,
                vector<BlockData> &blockDataList, block_list_t &symbolList,
                uint32_t segmentSize, string setting);

        /**
         * Get the list of blocks required to do decode
         * @param codingScheme Coding Scheme
         * @param blockStatus A bool array containing the status of the OSD
         * @param segmentSize Segment size
         * @param setting Setting for the coding scheme
         * @return List of block ID and the required offset length
         */

        block_list_t getRequiredBlockSymbols(CodingScheme codingScheme,
                vector<bool> blockStatus, uint32_t segmentSize, string setting);

        /**
         * Get the number of blocks that the scheme uses
         * @param codingScheme Coding Scheme
         * @param setting Setting for the coding scheme
         * @return number of blocks
         */

        uint32_t getNumberOfBlocks(CodingScheme codingScheme, string setting);

        /**
         * Get the list of blocks required to do repair
         * @param codingScheme Coding Scheme
         * @param failedBlocks Blocks that require repair
         * @param blockStatus A bool array containing the status of the OSD
         * @param segmentSize Segment size
         * @param setting Setting for the coding scheme
         * @return List of block ID and the required offset length
         */

        block_list_t getRepairBlockSymbols(CodingScheme codingScheme,
                vector<uint32_t> failedBlocks, vector<bool> blockStatus,
                uint32_t segmentSize, string setting);

        /**
         * Repair blocks from other blocks
         * @param codingScheme Coding Scheme
         * @param repairBlockIdList List of blocks that need repair
         * @param blockData a list of blockData that 
         * @param symbolList The symbol list obtained from getRepairBlockSymbols
         * @param segmentSize Segment size
         * @param setting Setting for the coding scheme
         * @return List of repaired BlockData structure
         */

        vector<BlockData> repairBlocks(CodingScheme codingScheme,
                vector<uint32_t> repairBlockIdList, vector<BlockData> &blockData,
                block_list_t &symbolList, uint32_t segmentSize, string setting);

        /**
         * Get the Coding segment according to the codingScheme specified
         * @param codingScheme Type of coding scheme
         * @return The Coding segment
         */

        Coding* getCoding(CodingScheme codingScheme);

    private:
        map<CodingScheme, Coding*> _codingWorker;
};

#endif
