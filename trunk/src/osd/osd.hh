/**
 * osd.hh
 */

#ifndef __OSD_HH__
#define __OSD_HH__
#include <stdint.h>
#include <vector>
#include "../common/metadata.hh"
#include "../protocol/message.hh"
#include "osd_communicator.hh"
#include "objectdata.hh"
#include "segmentdata.hh"
#include "segmentlocationcache.hh"
#include "storagemodule.hh"

/**
 * Central class of OSD
 * All functions of OSD are invoked here
 * Objects and Segments can be divided into trunks for transportation
 */

/**
 * Message Functions
 *
 * UPLOAD
 * 1. putObjectProcessor
 * 2. objectTrunkProcessor 	-> getOsdListRequest (MONITOR)
 * 									-> osdListProcessor
 * 							-> sendSegmentToOsd
 * 3. (other OSD) putSegmentProcessor
 * 4. (other OSD) segmentTrunkProcessor
 * 5. sendSegmentAck (PRIMARY OSD, CLIENT, MDS)
 *
 * DOWNLOAD
 * 1. getObjectProcessor 	-> getOsdListRequest (MDS)
 * 									-> osdListProcessor
 * 2. getSegmentRequest
 * 			-> (other OSD) getSegmentProcessor
 * 			-> (other OSD) sendSegmentToOsd
 * 			-> putSegmentProcessor
 * 			-> segmentTrunkProcessor
 * 	3. sendObjectToClient
 */

class Osd {
public:

	/**
	 * Constructor
	 */

	Osd();

	/**
	 * Destructor
	 */

	~Osd();

	/**
	 * Action when an OSD list is received
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	Object ID
	 * @param osdList 	Secondary OSD List
	 * @return Length of list if success, -1 if failure
	 */

	uint32_t osdListProcessor(uint32_t sockfd, uint64_t objectId,
			list<struct SegmentLocation> osdList);

	/**
	 * Action when a getObjectRequest is received
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	ID of the object to send
	 */

	void getObjectProcessor(uint32_t sockfd, uint64_t objectId);

	/**
	 * Action when a getSegmentRequest is received
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	ID of Object that the segment is belonged to
	 * @param segmentId ID of the segment to send
	 */

	void getSegmentProcessor(uint32_t sockfd, uint64_t objectId,
			uint32_t segmentId);

	/**
	 * Action when a put object request is received
	 * A number of trunks are expected to receive afterwards
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param length Object size, equals the total length of all the trunks
	 * @param trunkCount Total number of trunks that will be sent
	 * @return 0 if success, -1 if failure
	 */

	uint32_t putObjectProcessor(uint32_t sockfd, uint64_t objectId,
			uint32_t length, uint32_t trunkCount);

	/**
	 * Action when an object trunk is received
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param offset Offset of the trunk in the object
	 * @param length Length of trunk
	 * @param trunkId ID of the trunk in this object
	 * @param buf Pointer to buffer
	 * @return Length of trunk if success, -1 if failure
	 */

	uint32_t objectTrunkProcessor(uint32_t sockfd, uint64_t objectId,
			uint32_t offset, uint32_t length, uint32_t trunkId, char* buf);

	/**
	 * Action when a put object request is received
	 * A number of trunks are expected to receive afterwards
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Segment size, equals the total length of all the trunks
	 * @param trunkCount Total number of trunks that will be sent
	 * @return 0 if success, -1 if failure
	 */

	uint32_t putSegmentProcessor(uint32_t sockfd, uint64_t objectId,
			uint32_t segmentId, uint32_t length, uint32_t trunkCount);

	/**
	 * Action when a segment trunk is received
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param offset Offset of the trunk in the segment
	 * @param length Length of trunk
	 * @param buf Pointer to buffer
	 * @return Length of trunk if success, -1 if failure
	 */

	uint32_t segmentTrunkProcessor(uint32_t sockfd, uint64_t objectId,
			uint32_t segmentId, uint32_t offset, uint32_t length, char* buf);

	/**
	 * Action when a recovery request is received
	 * @param sockfd Socket descriptor of message source
	 * @return 0 if success, -1 if failure
	 */

	uint32_t recoveryProcessor(uint32_t sockfd);

	// getters

	/**
	 * Get a reference of OSDCommunicator
	 * @return Pointer to OSD communication module
	 */

	OsdCommunicator* getCommunicator();

	/**
	 * Get a reference of OSD Cache
	 * @return Pointer to OSD segment location cache
	 */
	SegmentLocationCache* getSegmentLocationCache();

private:

	/**
	 * Retrieve a segment from the storage
	 * @param objectId ID of the object that the segment is belonged to
	 * @param segmentId Target Segment ID
	 * @return SegmentData structure
	 */

	struct SegmentData getSegmentFromStroage(uint64_t objectId,
			uint32_t segmentId);

	/**
	 * Save a segment to storage
	 * @param segmentData a SegmentData structure
	 * @return Length of segment if success, -1 if failure
	 */

	uint32_t saveSegmentToStorage(SegmentData segmentData);

	/**
	 * Perform degraded read of an object
	 * @param objectId ID of the object to read
	 * @return an ObjectData structure
	 */

	struct ObjectData degradedRead(uint64_t objectId);

	/**
	 * Stores the list of OSDs that store a certain segment
	 */

	SegmentLocationCache* _segmentLocationCache;

	/**
	 * Handles communication with other components
	 */

	OsdCommunicator* _osdCommunicator;

	/**
	 * Handles the storage layer
	 */

	StorageModule* _storageModule;

	/**
	 * Handles coding and decoding
	 */

	CodingModule* _codingModule;

//	Coding _cunit; // encode & decode done here
	uint32_t _osdId;

};
#endif
