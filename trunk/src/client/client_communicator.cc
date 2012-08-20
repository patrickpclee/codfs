#include <iostream>
#include <thread>
#include "client_communicator.hh"
#include "client.hh"

#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"
#include "../protocol/metadata/listdirectoryrequest.hh"
#include "../protocol/metadata/uploadfilerequest.hh"
#include "../protocol/metadata/downloadfilerequest.hh"
#include "../protocol/transfer/getobjectrequest.hh"
#include "../protocol/transfer/getobjectreadymsg.hh"
#include "../protocol/transfer/getobjectreplymsg.hh"

/**
 * @brief	Send List Folder Request to MDS (Blocking)
 *
 * 1. Create List Directory Request Message \n
 * 2. Use Future to Wait until Reply is Back
 */
extern Client* client;


vector<FileMetaData> ClientCommunicator::listFolderData(uint32_t clientId,
		string path) {
	uint32_t mdsSockFd = getMdsSockfd();
	ListDirectoryRequestMsg* listDirectoryRequestMsg =
			new ListDirectoryRequestMsg(this, clientId, mdsSockFd, path);
	listDirectoryRequestMsg->prepareProtocolMsg();

//	future<vector<FileMetaData> > folderData =
//			listDirectoryRequestMsg->getFolderDataFuture();

	addMessage(listDirectoryRequestMsg, true);
	MessageStatus status = listDirectoryRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<FileMetaData> fileMetaData =
				listDirectoryRequestMsg->getFolderData();
		waitAndDelete(listDirectoryRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "List Directory Request Failed");
		exit(-1);
	}
	return {};
}

struct FileMetaData ClientCommunicator::uploadFile(uint32_t clientId,
		string path, uint64_t fileSize, uint32_t numOfObjs,
		CodingScheme codingScheme, string codingSetting) {

	uint32_t mdsSockFd = getMdsSockfd();
	UploadFileRequestMsg* uploadFileRequestMsg = new UploadFileRequestMsg(this,
			mdsSockFd, clientId, path, fileSize, numOfObjs, codingScheme,
			codingSetting);
	uploadFileRequestMsg->prepareProtocolMsg();

	addMessage(uploadFileRequestMsg, true);
	MessageStatus status = uploadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = uploadFileRequestMsg->getFileId();
		fileMetaData._objectList = uploadFileRequestMsg->getObjectList();
		fileMetaData._primaryList = uploadFileRequestMsg->getPrimaryList();
		waitAndDelete(uploadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Upload File Request Failed");
		exit(-1);
	}
	return {};
}


struct FileMetaData ClientCommunicator::downloadFile(uint32_t clientId, uint32_t fileId){
	uint32_t mdsSockFd = getMdsSockfd();
	DownloadFileRequestMsg* downloadFileRequestMsg = new DownloadFileRequestMsg(this, mdsSockFd, clientId, fileId);
	downloadFileRequestMsg->prepareProtocolMsg();

	addMessage(downloadFileRequestMsg, true);
	MessageStatus status = downloadFileRequestMsg->waitForStatusChange();

	if(status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._objectList = downloadFileRequestMsg->getObjectList();
		fileMetaData._primaryList = downloadFileRequestMsg->getPrimaryList();
		waitAndDelete(downloadFileRequestMsg);
		return fileMetaData;
	}else{
		debug("%s\n", "Download File Request Failed");
		exit(-1);
	}
	return {};
}

struct ObjectData ClientCommunicator::getObject(uint32_t clientId, uint32_t dstSockfd, uint64_t objectId) {
	uint32_t objectSize = 0;
	uint32_t chunkCount = 0;
	uint32_t requestId = 0;
	struct ObjectData objectData {};

	// step 1. CLIENT -> OSD : GetObjectRequest.
	GetObjectRequestMsg* getObjectRequestMsg = new GetObjectRequestMsg(this,dstSockfd,objectId);

	getObjectRequestMsg->prepareProtocolMsg();
	addMessage(getObjectRequestMsg,true);

	MessageStatus status = getObjectRequestMsg->waitForStatusChange();

	if(status == READY){
		objectSize = getObjectRequestMsg->getObjectSize();
		chunkCount = getObjectRequestMsg->getChunkCount();
		requestId = getObjectRequestMsg->getRequestId();
		waitAndDelete(getObjectRequestMsg);
	} else {
		debug("%s\n", "Get Object Request Failed");
		exit(-1);
	}
	debug("%s\n", "Get Object Request ACK-ed");

	client->updatePendingObjectChunkMap(objectId,chunkCount);


	// step 2. CLIENT -> OSD : GetObjectReady.
	GetObjectReadyMsg* getObjectReadyMsg = new GetObjectReadyMsg(this, requestId, dstSockfd, objectId);
	getObjectReadyMsg->prepareProtocolMsg();
	addMessage(getObjectReadyMsg);

	// to be implemented

	while(client->getPendingChunkCount(objectId) != 0){}
	objectData = client->ObjectManipulation(objectId,objectSize);

	return objectData;
}

/*
(replaced by Communicator::sendObject)

void ClientCommunicator::putObject(uint32_t clientId, uint32_t dstOsdSockfd,
		struct ObjectData objectData, CodingScheme codingScheme,
		string codingSetting) {

	const uint64_t totalSize = objectData.info.objectSize;
	const uint64_t objectId = objectData.info.objectId;
	char* buf = objectData.buf;

	const uint32_t chunkCount = ((totalSize - 1) / _chunkSize) + 1;

	// Step 1 : Send Init message (wait for reply)

	putObjectInit(clientId, dstOsdSockfd, objectId, totalSize, chunkCount,
			codingScheme, codingSetting);
	debug("%s\n", "Put Object Init ACK-ed");

	// Step 2 : Send data chunk by chunk

	uint64_t byteToSend = 0;
	uint64_t byteProcessed = 0;
	uint64_t byteRemaining = totalSize;

	while (byteProcessed < totalSize) {

		if (byteRemaining > _chunkSize) {
			byteToSend = _chunkSize;
		} else {
			byteToSend = byteRemaining;
		}

		putObjectData(clientId, dstOsdSockfd, objectId, buf, byteProcessed,
				byteToSend);
		byteProcessed += byteToSend;
		byteRemaining -= byteToSend;

	}

	// Step 3: Send End message

	putObjectEnd(clientId, dstOsdSockfd, objectId);

	// free buf
	MemoryPool::getInstance().poolFree(objectData.buf);

	cout << "Put Object ID = " << objectId << " Finished" << endl;

}
*/


//
// TODO: DUMMY CONNECTION FOR NOW
//

void ClientCommunicator::connectToMds() {
	uint16_t port = 50000;
	string ip = "127.0.0.1";
	ComponentType connectionType = MDS;

	connectAndAdd(ip, port, connectionType);

	return;
}

void ClientCommunicator::connectToOsd() {
	uint16_t port = 52000;
	string ip = "127.0.0.1";
	ComponentType connectionType = OSD;

	connectAndAdd(ip, port, connectionType);

	return;
}
