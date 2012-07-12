#ifndef __OSD_METADATA_MODULE_HH__
#define __OSD_METADATA_MODULE_HH__

#include <stdint.h>

#include "../common/metadata.hh"

#include "../cache/osdmetadatacache.hh"

class OsdMetaDataModule {
	public:
	private:
		OsdMetaDataCache* _osdMetaDataCache;
};

#endif
