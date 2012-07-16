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

class Osd {
public:

	Osd();
	~Osd();
	OsdCommunicator* getOsdCommunicator();
	SegmentLocationCache* getCache();

	list<uint32_t> secOsdListHandler(uint64_t objectId, list<uint32_t> osdList);

	ObjectData getObjectHandler(uint64_t objectId);
	SegmentData getSegmentHandler(uint64_t objectId, uint32_t segmentId);

	uint32_t objectTrunkHandler(uint64_t objectId, uint32_t offset,
			uint32_t length, vector<unsigned char> buf);
	uint32_t segmentTrunkHandler(uint64_t objectId, uint32_t segmentId,
			uint32_t offset, uint32_t length, vector<unsigned char> buf);

	uint32_t recoveryHandler (uint64_t objectId);

private:

	list<SegmentData> encodeObjectToSegment(ObjectData objectData);
	ObjectData decodeSegmentToObject(uint64_t objectId,
			list<SegmentData> segmentData);

	uint32_t sendAckToMds(uint64_t objectId, uint32_t segmentId);
	uint32_t sendAckToClient(uint32_t fileId);

	uint32_t getSegmentRequest(uint64_t objectId, uint32_t segmentId);
	uint32_t getSecOsdListRequest(uint64_t objectId);
	SegmentData getSegmentFromStroage(uint64_t objectId, uint32_t segmentId);

	uint32_t sendSegmentToOsd(SegmentData segmentData);
	uint32_t sendObjectToClient (ObjectData objectData);
	uint32_t saveSegmentToStorage(SegmentData segmentData);

	uint32_t degradedRead (uint64_t objectId);
	uint32_t reportOsdFailure (uint32_t osdId);

	SegmentLocationCache* _segmentLocationCache;
	OsdCommunicator* _osdCommunicator;
	StorageModule* _storageModule;

//	Coding _cunit; // encode & decode done here
//	OsdInfo _info;

};
#endif
