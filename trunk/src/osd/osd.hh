#ifndef __OSD_HH__
#define __OSD_HH__
#include <stdint.h>
class Osd {
public:
	uint32_t requestSegment(Cache _cache, uint32_t objectId);
	uint32_t transSegment(	Cache _cache, 
			uint32_t objectId, 
			SegmentMetaData[] segment_metadata_list );

private:
	uint32_t cacheObjectInfo (ObjectMetaData obj_info);
	uint32_t cacheSecondaryList (uint32_t[] secondary_node_list);

	uint32_t[] secondaryOsdListRequest (uint32_t objectId);

	Coding _cunit; // encode & decode done here
	OsdInfo _info;
	Cache _cache;
	DataStorage _localstore

		class DataStorage {
			unit32_t storeSegment(	uint32_t objectId, 
					SegmentMetaData seg_info, 
					char[] segment )  ;
			char[] getSegment( uint32_t objectId, SegmentMetaData seg_info) ;
		}

	Communicator _communicator;
}
#endif
