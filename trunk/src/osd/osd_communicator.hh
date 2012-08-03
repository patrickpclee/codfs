/**
 * osd_communicator.hh
 */

#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "../communicator/communicator.hh"
#include "segmentlocationcache.hh"

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

	void replyPutObjectInit(uint32_t requestId,
			uint32_t connectionId, uint64_t objectId);

	/**
	 * Reply PutObjectEndRequest
	 * @param requestId Request ID
	 * @param connectionId Connection ID
	 * @param objectId Object ID
	 */

	void replyPutObjectEnd(uint32_t requestId,
			uint32_t connectionId, uint64_t objectId);

	/**
	 * Send a failure report to MDS / Monitor
	 * @param osdId ID of the OSD that failed
	 * @return 0 if success, -1 if failure
	 */

	uint32_t reportOsdFailure(uint32_t osdId);

	/**
	 * Send a segment to another OSD
	 * @param sockfd Socket Descriptor of the destination
	 * @param segmentData SegmentData structure
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendSegment(uint32_t sockfd, struct SegmentData segmentData);

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
	 * @return List of OSD ID that should contain the object
	 */

	vector<struct SegmentLocation> getOsdListRequest(uint64_t objectId,
			ComponentType dstComponent);

	/**
	 * Send an acknowledgement to inform the dstComponent that the segment is stored
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId ID of the segment received and stored
	 * @param dstComponent Type of the component to ACK
	 * @return 0 if success, -1 if failure
	 */
	uint32_t sendSegmentAck(uint64_t objectId, uint32_t segmentId,
			ComponentType dstComponent);

	/**
	 * Override
	 * Aanalyze the MsgHeader and create the corresponding Message class
	 * Execute message.handle() in a separate thread
	 * @param buf Pointer to the buffer holding the Message
	 * @param sockfd Socket Descriptor of incoming connection
	 */

	// void dispatch(char* buf, uint32_t sockfd);
private:
};

#endif
