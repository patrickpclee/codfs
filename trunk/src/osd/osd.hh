/**
 * osd.hh
 */

#ifndef __OSD_HH__
#define __OSD_HH__
#include <stdint.h>
#include "../cache/cache.hh"
#include "../common/metadata.hh"
#include "osd_communicator.hh"

/**
 * Central class of OSD
 * All functions of OSD are invoked here
 */

class Osd {
public:
	Osd();
	~Osd();
	OsdCommunicator* getOsdCommunicator ();
//	uint32_t requestSegment(Cache _cache, uint32_t objectId);
//	uint32_t transSegment(Cache _cache, uint32_t objectId,
//			SegmentMetaData segment_metadata_list[]);

private:
//	uint32_t cacheObjectInfo(ObjectMetaData obj_info);
//	uint32_t cacheSecondaryList(uint32_t secondary_node_list[]);
//	uint32_t* secondaryOsdListRequest(uint32_t objectId);

	Cache* _cache;
	OsdCommunicator* _osdCommunicator;

//	Coding _cunit; // encode & decode done here
//	OsdInfo _info;
//	StorageModule _storageModule;

};
#endif
