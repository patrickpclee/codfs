#include <cstdio>
#include "mds.hh"

Mds::Mds()
{
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
}

uint32_t Mds::uploadFileHandler (uint32_t clientId, string dstPath, uint32_t numOfObjs)
{
	vector<uint64_t> objectList(numOfObjs);
	vector<uint32_t> primaryList(numOfObjs);
	uint32_t fileId = 0;

	_nameSpaceModule->createFile(clientId, dstPath);
	fileId = _metaDataModule->createFile(dstPath);

	objectList = newObjectList(numOfObjs);
	_metaDataModule->saveObjectList(fileId,objectList);
	
	primaryList = askPrimaryList(numOfObjs);

	_mdsCommunicator->sendObjectandPrimaryList (clientId, objectList, primaryList);

	return fileId;

}

void Mds::uploadObjectAckHandler (uint32_t clientId, uint32_t fileId, uint64_t objectId, vector<uint32_t> osdIdList)
{
	_metaDataModule->saveNodeList(objectId, osdIdList);
	_metaDataModule->setPrimary(objectId, osdIdList[0]);

	return ;
}

void Mds::downloadFileHandler (uint32_t clientId, string dstPath)
{
	uint32_t fileId = _metaDataModule->lookupFileId(dstPath);
	return downloadFileProcess(clientId, fileId, dstPath);
}

void Mds::downloadFileHandler (uint32_t clientId, uint32_t fileId)
{
	string path = _metaDataModule->lookupFilePath(fileId);
	return downloadFileProcess(clientId, fileId, path);
}

void Mds::downloadFileProcess (uint32_t clientId, uint32_t fileId, string path)
{
	vector<uint64_t> objectList;
	vector<uint32_t> primaryList;

	_nameSpaceModule->openFile(clientId, path);
	_metaDataModule->openFile(clientId, fileId);
	objectList = _metaDataModule->readObjectList(fileId);
	
	vector<uint64_t>::iterator it;
	uint32_t primaryId;

	for (it = objectList.begin(); it < objectList.end(); ++it){
		primaryId = _metaDataModule->getPrimary(*it);
		primaryList.push_back(primaryId);	
	}

	_mdsCommunicator->sendObjectandPrimaryList(clientId, objectList, primaryList);
	
	return ;
}

void Mds::secondaryNodeListHandler (uint32_t osdId, uint64_t objectId)
{
	vector<uint32_t>nodeList;

	nodeList = _metaDataModule->readNodeList(objectId);
	_mdsCommunicator->sendNodeList(osdId, objectId, nodeList);

	return ;
}

void Mds::primaryFailureHandler(uint32_t clientId, uint32_t osdId, uint64_t objectId, FailureReason reason)
{
	uint32_t actingPrimary = _metaDataModule->selectActingPrimary(objectId ,osdId);
	_mdsCommunicator->reportFailure(osdId,reason);
	_mdsCommunicator->sendPrimary(clientId,objectId,actingPrimary);

	return ;
}

int main (void)
{
	printf ("MDS\n");
	return 0;
}
