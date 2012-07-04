#ifndef __MDS_HH__
#define __MDS_HH__

#include <stdint.h>
#include "metadata.hh"

class Mds {
public:
	uint32_t uploadFileHandler (char dstPath[]);
	uint32_t downloadFileHandler (char dstPath[]);
	uint32_t downloadFileHandler (uint32_t fileId);

	uint32_t listFolderHandler (char path[]);

	uint32_t secondaryNodeListHandler (uint64_t objectId);

	uint32_t primaryFailureHandler (uint32_t osdId);
	uint32_t secondaryFailureHandler (uint32_t osdId);

	uint32_t osdObjectListHandler (uint32_t osdId);

	uint32_t nodeListUpdateHandler (uint64_t objectId, uint32_t osdIdList[]);
private:
	// Ask Monitor for Primary Node List
	uint32_t* requestPrimaryNodeList (uint32_t nObjects);

	// Send Primary Node List
	uint32_t sendPrimaryNodeList (uint32_t clientId, uint32_t fileId, uint32_t primaryNodeList[]);

	// Send Secondary Node List
	uint32_t sendSecondaryNodeList (uint32_t osdId, uint64_t objectId, uint32_t SecondaryNodeList[]);

	void updateOsdHealth (uint32_t osdId, uint32_t health);

//	MdsInfo _info;
//	Communicator _communicator;
//	MetaDataModule _metaDataModule;
//	NameSpaceModule _nameSpaceModule;
};
#endif
