#include <iostream>
#include "client_communicator.hh"

#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/uploadfilerequest.hh"
#include "../protocol/putobjectinitrequest.hh"
#include "../protocol/putobjectendrequest.hh"
#include "../protocol/objectdatamsg.hh"


/**
 * @brief	Send List Folder Request to MDS (Blocking)
 *
 * 1. Create List Directory Request Message \n
 * 2. Use Future to Wait until Reply is Back
 */
vector<FileMetaData> ClientCommunicator::listFolderData(uint32_t clientId,
		string path) {
	ListDirectoryRequestMsg* listDirectoryRequestMsg =
			new ListDirectoryRequestMsg(this, clientId, getMdsSockfd(), path);
	listDirectoryRequestMsg->prepareProtocolMsg();

//	future<vector<FileMetaData> > folderData =
//			listDirectoryRequestMsg->getFolderDataFuture();

	addMessage(listDirectoryRequestMsg, true);
	MessageStatus status = listDirectoryRequestMsg->waitForStatusChange();

	if(status == READY) {
		vector <FileMetaData> fileMetaData = listDirectoryRequestMsg->getFolderData();
		waitAndDelete (listDirectoryRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n","List Directory Request Failed");
		exit (-1);
	}
	return {};
}

struct FileMetaData ClientCommunicator::uploadFile (uint32_t clientId, string path, uint64_t fileSize, uint32_t numOfObjs)
{
	UploadFileRequestMsg* uploadFileRequestMsg = new UploadFileRequestMsg (this, getMdsSockfd(), clientId, path, fileSize, numOfObjs);	
	uploadFileRequestMsg->prepareProtocolMsg();

	addMessage(uploadFileRequestMsg, true);
	MessageStatus status = uploadFileRequestMsg->waitForStatusChange();

	if(status == READY) {
		struct FileMetaData fileMetaData {};
		fileMetaData._id = uploadFileRequestMsg->getFileId();
		fileMetaData._objectList = uploadFileRequestMsg->getObjectList();
		fileMetaData._primaryList = uploadFileRequestMsg->getPrimaryList();
		waitAndDelete (uploadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n","List Directory Request Failed");
		exit (-1);
	}
	return {};
}

void ClientCommunicator::putObject(uint32_t clientId, uint32_t dstOsdSockfd,
		struct ObjectData objectData) {

	const uint64_t totalSize = objectData.info.objectSize;
	const uint64_t objectId = objectData.info.objectId;
	char* buf = objectData.buf;

	const uint32_t chunkCount = ((totalSize - 1) / _chunkSize) + 1;

	// Step 1 : Send Init message (wait for reply)

	putObjectInit(clientId, dstOsdSockfd, objectId, totalSize, chunkCount);
	debug ("%s\n", "Put Object Init ACK-ed");

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

//
// PRIVATE FUNCTIONS
//

void ClientCommunicator::putObjectInit(uint32_t clientId, uint32_t dstOsdSockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount) {

	// Step 1 of the upload process

	PutObjectInitRequestMsg* putObjectInitRequestMsg = new PutObjectInitRequestMsg(this,
			dstOsdSockfd, objectId, length, chunkCount);

	putObjectInitRequestMsg->prepareProtocolMsg();
	addMessage(putObjectInitRequestMsg, true);

	MessageStatus status = putObjectInitRequestMsg->waitForStatusChange();
	if(status == READY) {
		waitAndDelete (putObjectInitRequestMsg);
		return;
	} else {
		debug("%s\n", "Put Object Init Failed");
		exit (-1);
	}

}

void ClientCommunicator::putObjectData(uint32_t clientID, uint32_t dstOsdSockfd,
		uint64_t objectId, char* buf, uint64_t offset, uint32_t length) {

	// Step 2 of the upload process
	ObjectDataMsg* objectDataMsg = new ObjectDataMsg(this, dstOsdSockfd, objectId, offset, length);

	objectDataMsg->prepareProtocolMsg();
	objectDataMsg->preparePayload(buf + offset, length);

	addMessage(objectDataMsg, false);
}

void ClientCommunicator::putObjectEnd(uint32_t clientId, uint32_t dstOsdSockfd,
		uint64_t objectId) {

	// Step 3 of the upload process

	PutObjectEndRequestMsg* putObjectEndRequestMsg = new PutObjectEndRequestMsg (this, dstOsdSockfd, objectId);

	putObjectEndRequestMsg->prepareProtocolMsg();
	addMessage(putObjectEndRequestMsg, true);

	debug ("%s\n", "before waitForStatusChange");
	MessageStatus status = putObjectEndRequestMsg->waitForStatusChange();
	if(status == READY) {
		debug ("%s\n", "status == READY");
		waitAndDelete (putObjectEndRequestMsg);
		debug ("%s\n", "msg deleted");
		return;
	} else {
		debug("%s\n", "Put Object Init Failed");
		exit (-1);
	}
}

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
