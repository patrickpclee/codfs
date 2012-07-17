/**
 * mds.cc
 */

#include <cstdio>
#include "mds.hh"

/**
 * Initialise MDS Communicator and MetaData Modules
 */
Mds::Mds()
{
	_metaDataModule = new MetaDataModule();
	_nameSpaceModule = new NameSpaceModule();
	_mdsCommunicator = new MdsCommunicator();
}

/**
 * @brief	Handle File Upload Request From Client
 *
 * 1. Create File in the Name Space (Directory Tree) \n
 * 2. Create File Meta Data (Generate File ID) \n
 * 3. Generate Object IDs \n
 * 4. Ask Monitor for Primary list \n
 * 5. Reply with Object and Primary List
 */
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

	_mdsCommunicator->sendObjectandPrimaryList (clientId, fileId, objectList, primaryList);

	return fileId;
}

/**
 * @brief	Handle Upload Object Acknowledgement from Primary
 *
 * 1. Save the Node List of the object \n
 * 2. Set the Primary for the object
 */
void Mds::uploadObjectAckHandler (uint32_t clientId, uint32_t fileId, uint64_t objectId, vector<uint32_t> osdIdList)
{
	_metaDataModule->saveNodeList(objectId, osdIdList);
	_metaDataModule->setPrimary(objectId, osdIdList[0]);

	return ;
}

/**
 * @brief	Handle Download File Request from Client (Request with Path)
 */
void Mds::downloadFileHandler (uint32_t clientId, string dstPath)
{
	uint32_t fileId = _metaDataModule->lookupFileId(dstPath);
	return downloadFileProcess(clientId, fileId, dstPath);
}

/**
 * @brief	Handle Download File Request from Client (Request with File ID)
 */
void Mds::downloadFileHandler (uint32_t clientId, uint32_t fileId)
{
	string path = _metaDataModule->lookupFilePath(fileId);
	return downloadFileProcess(clientId, fileId, path);
}

/**
 * @brief	Process the Download Request
 *
 * @param	clientId	ID of the client
 * @param	fileId		ID of the File
 * @param	path		Path of the File
 *
 * 1. Open File in the Name Space \n
 * 2. Open File Metadata \n
 * 3. Read Object List from the Metadata Module \n
 * 4. Read Primary Node ID for each Object \n
 * 5. Reply with Object and Primary List
 */
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

	_mdsCommunicator->sendObjectandPrimaryList(clientId, fileId, objectList, primaryList);
	
	return ;
}

/**
 * @brief	Handle the Secondary Node List Request from OSDs
 *
 * @param	osdId	ID of the OSD Requesting
 * @param	objectID	ID of the Object
 */
void Mds::secondaryNodeListHandler (uint32_t osdId, uint64_t objectId)
{
	vector<uint32_t>nodeList;

	nodeList = _metaDataModule->readNodeList(objectId);
	_mdsCommunicator->sendNodeList(osdId, objectId, nodeList);

	return ;
}

/**
 * @brief	Handle Primary Node Failure Report from Client
 *
 * @param	clientId	ID of the Client Reporting
 * @param	osdId		ID of the Failed OSD
 * @param	objectId	ID of the Failed Object
 * @param	reason		Reason of the Failure (Default to Node Failure)
 *
 * 1. Select Acting Primary \n
 * 2. Report Node Failure to Monitor \n
 * 3. Reply with Acting Primary ID \n
 */
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
