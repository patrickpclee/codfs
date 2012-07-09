#include <cstdio>
#include "mds.hh"

Mds::Mds()
{
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
}

uint32_t Mds::uploadFileHandler (string dstPath,uint32_t clientId, uint32_t numOfObjs)
{
	vector<uint64_t> objectList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;
	fileId = _nameSpaceModule->createFile(dstPath,clientId);
	_metaDataModule->createFile(fileId,dstPath);

	objectList = newObjectList(numOfObjs);
	_metaDataModule->saveObjectList(fileId,objectList);
	
	primaryList = askPrimaryList(numOfObjs);

	_mdsCommunicator->sendObjectandPrimaryList (clientId, objectList, primaryList);

	return fileId;

}

void Mds::uploadObjectAckHandler (uint32_t fileId, uint64_t objectId, vector<uint32_t> osdIdList)
{
	_metaDataModule->saveNodeList(objectId, osdIdList);
	_metaDataModule->setPrimary(objectId, osdIdList[0]);
}

int main (void)
{
	printf ("MDS\n");
	return 0;
}

