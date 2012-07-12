#ifndef __OSD_METADATA_MODULE_HH__
#define __OSD_METADATA_MODULE_HH__

#include <stdint.h>

#include "osdmetadatacache.hh"

#include "../common/metadata.hh"

class OsdMetaDataModule {
	public:
	private:
		OsdMetaDataCache* _osdMetaDataCache;
};

#endif
