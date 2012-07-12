#ifndef __MDS_HH__
#define __MDS_HH__

#include "metadatamodule.hh"
#include "namespacemodule.hh"
#include "mds_communicator.hh"

#include <stdint.h>
#include <string.h>
#include <vector>

class Mds {
public:
	Mds();
	~Mds();
	uint32_t uploadFileHandler (uint32_t clientId, string dstPath, uint32_t numOfObjs);
	void uploadObjectAckHandler (uint32_t clientId, uint32_t fileId, uint64_t objectId, vector<uint32_t> osdIdList);

	void downloadFileHandler (uint32_t clientId, string dstPath);
	void downloadFileHandler (uint32_t clientId, uint32_t fileId);

	void secondaryNodeListHandler (uint32_t clientId, uint64_t objectId);

	uint32_t listFolderHandler (string path);

	void primaryFailureHandler (uint32_t clientId, uint32_t osdId, uint64_t objectId, FailureReason reason=UNREACHABLE);
	uint32_t secondaryFailureHandler (uint32_t osdId);

	uint32_t osdObjectListHandler (uint32_t osdId);

	uint32_t nodeListUpdateHandler (uint64_t objectId, uint32_t osdIdList[]);
private:
	vector<uint64_t> newObjectList (uint32_t numOfObjs);
	// Ask Monitor for Primary Node List
	vector<uint32_t> askPrimaryList (uint32_t numOfObjs);

	// Send Primary Node List
	//uint32_t sendPrimaryNodeList (uint32_t clientId, uint32_t fileId, uint32_t primaryNodeList[]);

	// Send Secondary Node List
	//uint32_t sendSecondaryNodeList (uint32_t osdId, uint64_t objectId, uint32_t SecondaryNodeList[]);

	void downloadFileProcess (uint32_t clientId, uint32_t fileId, string path);

//	MdsInfo _info;
//	Communicator _communicator;	

	MdsCommunicator* _mdsCommunicator;
	MetaDataModule* _metaDataModule;
	NameSpaceModule* _nameSpaceModule;
};
#endif
