/**
 * osd_communicator.hh
 */

#ifndef __OSD_COMMUNICATOR_HH__
#define __OSD_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
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
	void connectToMds ();

	/**
	 * Send a failure report to MDS / Monitor
	 * @param osdId ID of the OSD that failed
	 * @return 0 if success, -1 if failure
	 */

	uint32_t reportOsdFailure(uint32_t osdId);

	/**
	 * Send a segment to another OSD
	 * @param segmentData a SegmentData structure
	 * @param osdId ID of the destination OSD
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendSegmentToOsd(struct SegmentData segmentData, uint32_t osdId);

	/**
	 * Send an object to a client
	 * @param objectData an objectData structure
	 * @param clientId ID of the destination client
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendObjectToClient(struct ObjectData objectData,
			uint32_t clientId);

	/**
	 * Send a request to get a segment to other OSD
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId
	 * @return 0 if success, -1 if failure
	 */

	uint32_t getSegmentRequest(uint64_t objectId, uint32_t segmentId);

	/**
	 * Send a request to get the secondary OSD list of an object from MDS/Monitor
	 * @param objectId Object ID for query
	 * @return 0 if success, -1 if failure
	 */

	uint32_t getSecOsdListRequest(uint64_t objectId);

private:
};

#endif
