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
#include "../protocol/metadata/saveobjectlistrequest.hh"
#include "../protocol/transfer/putobjectinitrequest.hh"
#include "../protocol/transfer/putobjectinitreply.hh"
#include "../protocol/transfer/objecttransferendrequest.hh"
#include "../protocol/transfer/objecttransferendreply.hh"
#include "../protocol/transfer/objectdatamsg.hh"
#include "../protocol/transfer/getobjectrequest.hh"

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

struct FileMetaData ClientCommunicator::downloadFile(uint32_t clientId,
		uint32_t fileId) {
	uint32_t mdsSockFd = getMdsSockfd();
	DownloadFileRequestMsg* downloadFileRequestMsg = new DownloadFileRequestMsg(
			this, mdsSockFd, clientId, fileId);
	downloadFileRequestMsg->prepareProtocolMsg();

	addMessage(downloadFileRequestMsg, true);
	MessageStatus status = downloadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = downloadFileRequestMsg->getFileId();
		fileMetaData._path = downloadFileRequestMsg->getFilePath();
		fileMetaData._size = downloadFileRequestMsg->getSize();
		fileMetaData._objectList = downloadFileRequestMsg->getObjectList();
		fileMetaData._primaryList = downloadFileRequestMsg->getPrimaryList();
		waitAndDelete(downloadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Download File Request Failed");
		exit(-1);
	}
	return {};
}

struct FileMetaData ClientCommunicator::downloadFile(uint32_t clientId,
		string filePath) {
	uint32_t mdsSockFd = getMdsSockfd();
	DownloadFileRequestMsg* downloadFileRequestMsg = new DownloadFileRequestMsg(
			this, mdsSockFd, clientId, filePath);
	downloadFileRequestMsg->prepareProtocolMsg();

	addMessage(downloadFileRequestMsg, true);
	MessageStatus status = downloadFileRequestMsg->waitForStatusChange();

	if (status == READY) {
		struct FileMetaData fileMetaData { };
		fileMetaData._id = downloadFileRequestMsg->getFileId();
		fileMetaData._path = downloadFileRequestMsg->getFilePath();
		fileMetaData._size = downloadFileRequestMsg->getSize();
		fileMetaData._objectList = downloadFileRequestMsg->getObjectList();
		fileMetaData._primaryList = downloadFileRequestMsg->getPrimaryList();
		waitAndDelete(downloadFileRequestMsg);
		return fileMetaData;
	} else {
		debug("%s\n", "Download File Request Failed");
		exit(-1);
	}
	return {};
}

/**
 * @brief	Get File Info
 * TODO: Currently Doing Same as Download
 */
struct FileMetaData ClientCommunicator::getFileInfo(uint32_t clientId,
		uint32_t fileId) {
	return downloadFile(clientId, fileId);
}

/**
 *
 * @brief	Get File Info
 * TODO: Currently Doing Same as Download
 */
struct FileMetaData ClientCommunicator::getFileInfo(uint32_t clientId,
		string filePath) {
	return downloadFile(clientId, filePath);
}

void ClientCommunicator::saveObjectList(uint32_t clientId, uint32_t fileId,
		vector<uint64_t> objectList) {
	uint32_t mdsSockfd = getMdsSockfd();
	SaveObjectListRequestMsg* saveObjectListRequestMsg =
			new SaveObjectListRequestMsg(this, mdsSockfd, clientId, fileId,
					objectList);
	saveObjectListRequestMsg->prepareProtocolMsg();

	addMessage(saveObjectListRequestMsg);

	return;
}

void ClientCommunicator::replyPutObjectInit(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId) {

	PutObjectInitReplyMsg* putObjectInitReplyMsg = new PutObjectInitReplyMsg(
			this, requestId, connectionId, objectId);
	putObjectInitReplyMsg->prepareProtocolMsg();

	addMessage(putObjectInitReplyMsg);
}

void ClientCommunicator::replyPutObjectEnd(uint32_t requestId,
		uint32_t connectionId, uint64_t objectId) {

	ObjectTransferEndReplyMsg* putObjectEndReplyMsg =
			new ObjectTransferEndReplyMsg(this, requestId, connectionId,
					objectId);
	putObjectEndReplyMsg->prepareProtocolMsg();

	debug("Reply put object end for ID: %" PRIu64 "\n", objectId);
	addMessage(putObjectEndReplyMsg);
}

void ClientCommunicator::requestObject(uint32_t dstSockfd, uint64_t objectId) {
	GetObjectRequestMsg* getObjectRequestMsg = new GetObjectRequestMsg(this,
			dstSockfd, objectId);

	getObjectRequestMsg->prepareProtocolMsg();
	addMessage(getObjectRequestMsg);
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
