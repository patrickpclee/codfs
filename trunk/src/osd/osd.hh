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
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "segmentlocation.hh"
#include "storagemodule.hh"
#include "codingmodule.hh"

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

	Osd(string configFilePath);

	/**
	 * Destructor
	 */

	~Osd();

	/**
	 * Action when an OSD list is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	Object ID
	 * @param osdList 	Secondary OSD List
	 * @return Length of list if success, -1 if failure
	 */

	uint32_t osdListProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, vector<struct SegmentLocation> osdList);

	// DOWNLOAD

	/**
	 * Action when a getObjectRequest is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	ID of the object to send
	 */

	void getObjectRequestProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId);

	/**
	 * Action when a getSegmentRequest is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId 	ID of Object that the segment is belonged to
	 * @param segmentId ID of the segment to send
	 */

	void getSegmentRequestProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId);

	/**
	 * Action when a put object request is received
	 * A number of trunks are expected to receive afterwards
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param length Object size, equals the total length of all the trunks
	 * @param chunkCount number of chunks that will be received
	 * @param codingScheme Coding Scheme for the object
	 * @param setting Coding setting for the object
	 */

	void putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t length, uint32_t chunkCount,
			CodingScheme codingScheme, string setting);

	/**
	 * Action when a put object end is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 */

	void putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId);

	/**
	 * Action when an object trunk is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param offset Offset of the trunk in the object
	 * @param length Length of trunk
	 * @param buf Pointer to buffer
	 * @return Length of trunk if success, -1 if failure
	 */

	uint32_t putObjectDataProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint64_t offset, uint32_t length, char* buf);

	/**
	 * Action when a put object request is received
	 * A number of trunks are expected to receive afterwards
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param length Segment size, equals the total length of all the trunks
	 * @param chunkCount No of trunks to receive
	 */

	void putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId, uint32_t length,
			uint32_t chunkCount);

	/**
	 * Action when a segment trunk is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 * @param offset Offset of the trunk in the segment
	 * @param length Length of trunk
	 * @param buf Pointer to buffer
	 * @return Length of trunk if success, -1 if failure
	 */

	uint32_t putSegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId, uint32_t offset,
			uint32_t length, char* buf);

	/**
	 * Action when a put segment end is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param objectId Object ID
	 * @param segmentId Segment ID
	 */

	void putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t objectId, uint32_t segmentId);

	/**
	 * Action when a recovery request is received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 */

	void recoveryProcessor(uint32_t requestId, uint32_t sockfd);

	/**
	 * Action when a monitor requests a status update 
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 */
	void OsdStatUpdateRequestProcessor(uint32_t requestId, uint32_t sockfd);

	// getters

	/** 
	 * To get the current cpu load average in last 15 mins. If error, return
	 * infinity INF = (1<<29)
	 * @param idx 0 to get last one minute, 1 to get last 5 mins, 2 to get last 15 mins
	 * @return loading*100 to cast into integer
	 */
	uint32_t getCpuLoadavg(int idx);

	/**
	 * To get the free space of the current disk in MB
	 * @return free space in MB, if error, return 0
	 */
	uint32_t getFreespace();

	/**
	 * Get a reference of OSDCommunicator
	 * @return Pointer to OSD communication module
	 */

	OsdCommunicator* getCommunicator();

	/**
	 * Get a reference of OSD Cache
	 * @return Pointer to OSD segment location cache
	 */
	//SegmentLocationCache* getSegmentLocationCache();
	/**
	 * Get the ID
	 * @return OSD ID
	 */

	uint32_t getOsdId();

	bool isSegmentReceived(uint64_t objectId, uint32_t segmentId);

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

	//SegmentLocationCache* _segmentLocationCache;
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

	// upload
	map<uint64_t, uint32_t> _pendingObjectChunk;
	map<uint64_t, struct CodingSetting> _codingSettingMap;

	// download
	map<uint64_t, uint32_t> _objectRequestCount;
	map<uint64_t, vector<struct SegmentData>> _receivedSegmentData;
	map<uint64_t, uint32_t> _pendingSegmentCount;
	map<uint64_t, vector<bool>> _receivedSegments;

	// upload / download
	map<string, uint32_t> _pendingSegmentChunk;
};
#endif
