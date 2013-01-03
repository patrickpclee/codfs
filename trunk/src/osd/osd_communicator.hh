/**
 * osd_communicator.hh
 */

#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "../common/metadata.hh"
#include "../common/blocklocation.hh"
#include "../communicator/communicator.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all OSD communications
 */

class OsdCommunicator: public Communicator {
public:

	/**
	 * Constructor
	 */

	OsdCommunicator();

	/**
	 * Destructor
	 */

	~OsdCommunicator();

	/**
	 * Reply PutSegmentInitRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 */

	void replyPutSegmentInit(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId);

	/**
	 * Reply PutBlockInitRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 */

	void replyPutBlockInit(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, uint32_t blockId);

	/**
	 * Reply PutSegmentEndRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 */

	void replyPutSegmentEnd(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId);

	/**
	 * Reply to PutBlockEndRequest / RecoveryBlockData
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 * @param waitOnRequestId (For recovery, specify a request id for reply)
	 */

	void replyPutBlockEnd(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId, uint32_t blockId, uint32_t waitOnRequestId = 0);

	/**
	 * Reply to MDS when the segment is cached
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param segmentId Segment ID
	 */

	void replyCacheSegment(uint32_t requestId, uint32_t connectionId,
			uint64_t segmentId);

	/**
	 * Report deleted segment for the last polling interval to the MDS
	 * @param segmentIdList A list of deleted segment ID
	 * @param osdId My OSD ID
	 */

	void reportDeletedCache(list<uint64_t> segmentIdList, uint32_t osdId);

	/**
	 * (to be implemented)
	 * Report a failure of OSD to monitor / MDS
	 * @param osdId Failed OSD ID
	 * @return 0 if success, -1 if failure
	 */

	uint32_t reportOsdFailure(uint32_t osdId);

	/**
	 * Send a block to another OSD
	 * @param sockfd Socket Descriptor of the destination
	 * @param blockData BlockData structure
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendBlock(uint32_t sockfd, struct BlockData blockData);

	/**
	 * Send a recovery block to another OSD
	 * @param requestId Request ID
	 * @param sockfd Socket Descriptor of the destination
	 * @param blockData BlockData structure
	 * @return 0 if success, -1 if failure
	 */
	uint32_t sendRecoveryBlock(uint32_t requestId, uint32_t sockfd,
			struct BlockData blockData);

	/**
	 * Send a request to get a block to other OSD
	 * @param osdId Target Osd ID
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 * @param symbols List of <offset,length> to obtain for the block
	 */

	void getBlockRequest(uint32_t osdId, uint64_t segmentId, uint32_t blockId,
			vector<offset_length_t> symbols);

	/**
	 * Send a request to get a recovery block to other OSD
	 * @param osdId Target Osd ID
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 * @param symbols List of <offset,length> to obtain for the block
	 * @return Received BlockData Structure
	 */

	BlockData getRecoveryBlock(uint32_t osdId, uint64_t segmentId,
			uint32_t blockId, vector<offset_length_t> symbols);

	/**
	 * Send a request to get the secondary OSD list of an segment from MDS/Monitor
	 * @param segmentId Segment ID for query
	 * @param dstComponent Type of the component to request (MDS / MONITOR)
	 * @param blockCount (optional) Request a specific number of OSD to hold data
	 * @return List of OSD ID that should contain the segment
	 */

	vector<struct BlockLocation> getOsdListRequest(uint64_t segmentId,
			ComponentType dstComponent, uint32_t blockCount, uint32_t primaryId,
			uint64_t blockSize);

	/**
	 * Send an acknowledgement to inform the dstComponent that the block is stored
	 * @param segmentId ID of the segment that the block is belonged to
	 * @param blockId ID of the block received and stored
	 * @param dstComponent Type of the component to ACK
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendBlockAck(uint64_t segmentId, uint32_t blockId,
			ComponentType dstComponent);

	/**
	 * Obtain the information about an segment from the MDS
	 * @param segmentId Segment ID
	 * @return SegmentTransferOsdInfo struct
	 */

	SegmentTransferOsdInfo getSegmentInfoRequest(uint64_t segmentId);

	/**
	 * Send acknowledgement to MDS when upload is complete
	 * @param segmentId Segment ID
	 * @param segmentSize Segment Size
	 * @param codingScheme Coding Scheme
	 * @param codingSetting Coding Setting
	 * @param nodeList List of OSD that saved blocks for the segment
	 * @param checksum Checksum of the segment
	 */

	void segmentUploadAck(uint64_t segmentId, uint32_t segmentSize,
			CodingScheme codingScheme, string codingSetting,
			vector<uint32_t> nodeList, string checksum);

	/**
	 * Register myself to the MONITOR
	 * @param selfIp My IP
	 * @param selfPort My Port
	 */

	void registerToMonitor(uint32_t selfIp, uint16_t selfPort);

	/**
	 * Request the status of OSD from MONITOR
	 * @param osdIdList List of OSD ID to request
	 * @return boolean array storing OSD health
	 */

	vector<bool> getOsdStatusRequest(vector<uint32_t> osdIdList);

	/**
	 * Acknowledge MDS for completing block repair
	 * @param segmentId Segment ID
	 * @param repairBlockList List of blocks to repair
	 * @param repairBlockOsdList List of OSD to store the repaired blocks
	 */

	void repairBlockAck(uint64_t segmentId, vector<uint32_t> repairBlockList,
			vector<uint32_t> repairBlockOsdList);
private:

	/**
	 * Initiate upload process to OSD (Step 1)
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 * @param length Size of the segment
	 * @param chunkCount Number of chunks that will be sent
	 */

	void putBlockInit(uint32_t sockfd, uint64_t segmentId, uint32_t blockId,
			uint32_t length, uint32_t chunkCount);

	/**
	 * Send an segment chunk to OSD (Step 2)
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 * @param buf Buffer containing the segment
	 * @param offset Offset of the chunk inside the buffer
	 * @param length Length of the chunk
	 */

	void putBlockData(uint32_t sockfd, uint64_t segmentId, uint32_t blockId,
			char* buf, uint64_t offset, uint32_t length);

	/**
	 * Finalise upload process to OSD (Step 3)
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param blockId Block ID
	 */

	void putBlockEnd(uint32_t sockfd, uint64_t segmentId, uint32_t blockId);

};

#endif
