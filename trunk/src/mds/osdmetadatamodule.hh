#ifndef __OSD_METADATA_MODULE_HH__
#define __OSD_METADATA_MODULE_HH__

#include <stdint.h>

#include "../common/metadata.hh"
#include "../storage/mongodb.hh"

class OsdMetaDataModule {
	public:
	private:
		string _collection;

		MongoDB* _osdMetaDataStorage;
//		OsdMetaDataCache* _osdMetaDataCache;
};

#endif
