#include <iostream>
#include <thread>
#include "client_communicator.hh"
#include "client.hh"

#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"
#include "../protocol/metadata/listdirectoryrequest.hh"
#include "../protocol/metadata/uploadfilerequest.hh"
#include "../protocol/metadata/deletefilerequest.hh"
#include "../protocol/metadata/downloadfilerequest.hh"
#include "../protocol/metadata/saveobjectlistrequest.hh"
#include "../protocol/metadata/setfilesizerequest.hh"
#include "../protocol/transfer/putobjectinitrequest.hh"
#include "../protocol/transfer/putobjectinitreply.hh"
#include "../protocol/transfer/objecttransferendrequest.hh"
#include "../protocol/transfer/objecttransferendreply.hh"
#include "../protocol/transfer/objectdatamsg.hh"
#include "../protocol/metadata/getobjectidlistrequest.hh"
#include "../protocol/transfer/getobjectrequest.hh"
#include "../protocol/nodelist/getosdlistrequest.hh"
#include "../protocol/nodelist/getosdlistreply.hh"
#include "../protocol/status/switchprimaryosdrequestmsg.hh"

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

void ClientCommunicator::deleteFile(uint32_t clientId, string path, uint32_t fileId) {
	uint32_t mdsSockFd = getMdsSockfd();
	DeleteFileRequestMsg* deleteFileRequestMsg = new DeleteFileRequestMsg(this, mdsSockFd, clientId, fileId, path);
	deleteFileRequestMsg->prepareProtocolMsg();
	addMessage(deleteFileRequestMsg, true);
	MessageStatus status = deleteFileRequestMsg->waitForStatusChange();

	if (status == READY){
		return ;
	} else {
		debug_yellow("Delete File Request Failed %s [%" PRIu32 "]\n", path.c_str(), fileId);
		exit(-1);
	}
	return ;
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

	addMessage(saveObjectListRequestMsg, true);

	MessageStatus status = saveObjectListRequestMsg->waitForStatusChange();

	if (status == READY) {
		return ;
	} else {
		debug("Save Object List Request Failed [%" PRIu32 "]\n",fileId);
		exit(-1);
	}
	return;
}

vector<struct ObjectMetaData> ClientCommunicator::getNewObjectList (uint32_t clientId, uint32_t numOfObjs)
{
	uint32_t mdsSockfd = getMdsSockfd();
	debug("Requesting New Object Id, Number of Objects %" PRIu32 "\n", numOfObjs);
	GetObjectIdListRequestMsg* getObjectIdListRequestMsg = new GetObjectIdListRequestMsg(this, mdsSockfd, clientId, numOfObjs);
	getObjectIdListRequestMsg->prepareProtocolMsg();

	addMessage(getObjectIdListRequestMsg,true);

	MessageStatus status = getObjectIdListRequestMsg->waitForStatusChange();

	if(status == READY) {
		vector<struct ObjectMetaData> objectMetaDataList;
		vector<uint32_t> primaryList = getObjectIdListRequestMsg->getPrimaryList();
		vector<uint64_t> objectList = getObjectIdListRequestMsg->getObjectIdList();
		for(uint32_t i = 0; i < primaryList.size(); ++i){
			struct ObjectMetaData tempObjectMetaData;
			tempObjectMetaData._id =  objectList[i];
			tempObjectMetaData._primary = primaryList[i];
			objectMetaDataList.push_back(tempObjectMetaData);
		}
		return objectMetaDataList;
	} else {
		debug("%s\n","Get New Object List Failed");
		exit(-1);
	}

	return {};
}

void ClientCommunicator::saveFileSize(uint32_t clientId, uint32_t fileId, uint64_t fileSize)
{
	uint32_t mdsSockfd = getMdsSockfd();
	SetFileSizeRequestMsg* setFileSizeRequestMsg = new SetFileSizeRequestMsg(this, mdsSockfd, clientId, fileId, fileSize);
	
	setFileSizeRequestMsg->prepareProtocolMsg();
	addMessage(setFileSizeRequestMsg);
	return ;
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

void ClientCommunicator::getOsdListAndConnect() {
	GetOsdListRequestMsg* getOsdListRequestMsg = new GetOsdListRequestMsg(this,
		getMonitorSockfd());
	getOsdListRequestMsg->prepareProtocolMsg();
	addMessage(getOsdListRequestMsg, true);
	MessageStatus status = getOsdListRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<struct OnlineOsd>& onlineList =
				getOsdListRequestMsg->getOsdList();
		for (uint32_t i = 0; i < onlineList.size(); i++)
			connectToOsd(onlineList[i].osdIp, onlineList[i].osdPort);
	}
	
}

uint32_t ClientCommunicator::switchPrimaryRequest(uint32_t clientId, uint64_t objectId) {
	uint32_t dstSockfd = -1;
	while (dstSockfd == (uint32_t) -1) {
		SwitchPrimaryOsdRequestMsg* msg = new
		SwitchPrimaryOsdRequestMsg(this, getMdsSockfd(), clientId, objectId);

		msg->prepareProtocolMsg();
		addMessage(msg, true);
		msg->waitForStatusChange();

		dstSockfd = getSockfdFromId(msg->getNewPrimaryOsdId());
	}
	return dstSockfd;
}
