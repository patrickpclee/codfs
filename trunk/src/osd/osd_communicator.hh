/**
 * osd_communicator.hh
 */

#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "segmentlocation.hh"
#include "../communicator/communicator.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all OSD communications
 */

class OsdCommunicator: public Communicator {
public:
	OsdCommunicator();
	~OsdCommunicator();
//	void listDirectoryRequest(uint32_t osdId, string directoryPath);

	/**
	 * Initiate a connection to the MDS
	 */

	void connectToMds();

	/**
	 * Reply PutObjectInitRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param objectId Object ID
	 */

	void replyPutObjectInit(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId);

	/**
	 * Reply PutSegmentInitRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void replyPutSegmentInit(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, uint32_t segmentId);

	/**
	 * Reply PutObjectEndRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param objectId Object ID
	 */

	void replyPutObjectEnd(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId);

	/**
	 * Send a failure report to MDS / Monitor
	 * @param osdId ID of the OSD that failed
	 * @return 0 if success, -1 if failure
	 */

	void replyPutSegmentEnd(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, uint32_t segmentId);

	uint32_t reportOsdFailure(uint32_t osdId);

	/**
	 * Send a segment to another OSD
	 * @param osdId My OSD ID
	 * @param sockfd Socket Descriptor of the destination
	 * @param segmentData SegmentData structure
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendSegment(uint32_t osdId, uint32_t sockfd,
			struct SegmentData segmentData);

	/**
	 * Send an object to a client
	 * @param sockfd Socket Descriptor of the destination
	 * @param objectData OjectData structure
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendObject(uint32_t sockfd, struct ObjectData objectData);

	/**
	 * Send a request to get a segment to other OSD
	 * @param connectionId ID of the target component connection
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId
	 * @return SegmentData structure
	 */

	struct SegmentData getSegmentRequest(uint32_t osdId, uint64_t objectId,
			uint32_t segmentId);

	/**
	 * Send a request to get the secondary OSD list of an object from MDS/Monitor
	 * @param objectId Object ID for query
	 * @param dstComponent Type of the component to request (MDS / MONITOR)
	 * @param segmentCount (optional) Request a specific number of OSD to hold data
	 * @return List of OSD ID that should contain the object
	 */

	vector<struct SegmentLocation> getOsdListRequest(uint64_t objectId,
			ComponentType dstComponent, uint32_t segmentCount = 0);

	/**
	 * Send an acknowledgement to inform the dstComponent that the segment is stored
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId ID of the segment received and stored
	 * @param dstComponent Type of the component to ACK
	 * @return 0 if success, -1 if failure
	 */
	uint32_t sendSegmentAck(uint64_t objectId, uint32_t segmentId,
			ComponentType dstComponent);

private:

	/**
	 * Initiate upload process to OSD (Step 1)
	 * @param osdId OSD ID
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Size of the object
	 * @param chunkCount Number of chunks that will be sent
	 */

	void putSegmentInit(uint32_t osdId, uint32_t sockfd, uint64_t objectId,
			uint32_t segmentId, uint32_t length, uint32_t chunkCount);

	/**
	 * Send an object chunk to OSD (Step 2)
	 * @param osdId OSD ID
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param buf Buffer containing the object
	 * @param offset Offset of the chunk inside the buffer
	 * @param length Length of the chunk
	 */

	void putSegmentData(uint32_t osdId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId, char* buf, uint64_t offset,
			uint32_t length);

	/**
	 * Finalise upload process to OSD (Step 3)
	 * @param clientId Client ID
	 * @param sockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void putSegmentEnd(uint32_t clientId, uint32_t sockfd, uint64_t objectId,
			uint32_t segmentId);
};

#endif
