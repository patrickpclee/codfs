#include <iostream>
#include "mds_communicator.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../protocol/metadata/listdirectoryreply.hh"
#include "../protocol/metadata/uploadfilereply.hh"
#include "../protocol/metadata/saveobjectlistreply.hh"
#include "../protocol/metadata/deletefilereply.hh"
#include "../protocol/nodelist/getprimarylistrequest.hh"
#include "../protocol/metadata/getobjectinforeply.hh"
#include "../protocol/metadata/downloadfilereply.hh"
#include "../protocol/metadata/getobjectidlistreply.hh"
#include "../protocol/metadata/renamefilereply.hh"
#include "../protocol/status/switchprimaryosdreplymsg.hh"
#include "../protocol/status/getosdstatusrequestmsg.hh"
#include "../protocol/status/recoverytriggerreply.hh"
//#include "../protocol/metadata/heresfilesize.hh"

extern ConfigLayer* configLayer;

MdsCommunicator::MdsCommunicator() {
	_serverPort = configLayer->getConfigInt("Communication>ServerPort");
}

/**
 * @brief	Reply With Folder Data
 */
void MdsCommunicator::replyFolderData(uint32_t requestId, uint32_t connectionId,
		string path, vector<FileMetaData> folderData) {
	ListDirectoryReplyMsg* listDirectoryReplyMsg = new ListDirectoryReplyMsg(
			this, requestId, connectionId, path, folderData);
	listDirectoryReplyMsg->prepareProtocolMsg();

	addMessage(listDirectoryReplyMsg);
	return;
}

/*
 void MdsCommunicator::replyFileSize(uint32_t requestId, uint32_t connectionId, uint32_t fileId, uint64_t fileSize){
 HeresFileSizeMsg* heresFileSizeMsg = new HeresFileSizeMsg(this, requestId, connectionId, fileId, fileSize);
 heresFileSizeMsg->prepareProtocolMsg();

 addMessage(heresFileSizeMsg);
 return;
 }
 */

/**
 * @brief	Ask Monitor for Primary List
 */
vector<uint32_t> MdsCommunicator::askPrimaryList(uint32_t numOfObjs) {
	vector<uint32_t> primaryList;
	for (uint32_t i = 0; i < numOfObjs; ++i)
		primaryList.push_back(52000 + (i % 2));
	return primaryList;
}

vector<uint32_t> MdsCommunicator::getPrimaryList(uint32_t sockfd,
		uint32_t numOfObjs) {
	GetPrimaryListRequestMsg* getPrimaryListRequestMsg =
			new GetPrimaryListRequestMsg(this, sockfd, numOfObjs);
	getPrimaryListRequestMsg->prepareProtocolMsg();

	addMessage(getPrimaryListRequestMsg, true);
	MessageStatus status = getPrimaryListRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<uint32_t> primaryList =
				getPrimaryListRequestMsg->getPrimaryList();
		return primaryList;
	}
	return {};
}

void MdsCommunicator::replyDeleteFile(uint32_t requestId, uint32_t connectionId,
		uint32_t fileId) {
	DeleteFileReplyMsg* deleteFileReplyMsg = new DeleteFileReplyMsg(this,
			requestId, connectionId, fileId);
	deleteFileReplyMsg->prepareProtocolMsg();
	addMessage(deleteFileReplyMsg);
	return;
}

/**
 * @brief	Reply Rename File
 *
 * @param	requestId	Request ID
 * @param	connectionId	Connection ID
 * @param	fileId	File Id
 * @param	path	File Path
 */
void MdsCommunicator::replyRenameFile(uint32_t requestId, uint32_t connectionId, uint32_t fileId, const string& path) {
	RenameFileReplyMsg* renameFileReplyMsg = new RenameFileReplyMsg(this, requestId, connectionId, fileId);
	renameFileReplyMsg->prepareProtocolMsg();
	addMessage(renameFileReplyMsg);
	return ;
}

void MdsCommunicator::display() {
	return;
}

void MdsCommunicator::replyObjectInfo(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, uint32_t objectSize, vector<uint32_t> nodeList,
		CodingScheme codingScheme, string codingSetting) {
	GetObjectInfoReplyMsg* getObjectInfoReplyMsg = new GetObjectInfoReplyMsg(
			this, requestId, connectionId, objectId, objectSize, nodeList,
			codingScheme, codingSetting);
	getObjectInfoReplyMsg->prepareProtocolMsg();

	debug("%s\n", "======================");
	getObjectInfoReplyMsg->printHeader();
	getObjectInfoReplyMsg->printProtocol();
	debug("%s\n", "======================");

	addMessage(getObjectInfoReplyMsg);
	return;
}

/**
 * @brief	Reply Save Object List Request
 */
void MdsCommunicator::replySaveObjectList(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId) {
	SaveObjectListReplyMsg* saveObjectListReplyMsg = new SaveObjectListReplyMsg(
			this, requestId, connectionId, fileId);
	saveObjectListReplyMsg->prepareProtocolMsg();
	addMessage(saveObjectListReplyMsg);
	return;
}

/**
 * @brief	Reply Object and Primary List to Client
 */
void MdsCommunicator::replyObjectandPrimaryList(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId, vector<uint64_t> objectList,
		vector<uint32_t> primaryList) {
	UploadFileReplyMsg* uploadFileReplyMsg = new UploadFileReplyMsg(this,
			requestId, connectionId, fileId, objectList, primaryList);
	uploadFileReplyMsg->prepareProtocolMsg();

	addMessage(uploadFileReplyMsg);
	return;
}

/**
 * @brief	Reply Download Information to Client
 *
 * File Size, Object List, Primary List, Checksum
 */
void MdsCommunicator::replyDownloadInfo(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId, string filePath,
		uint64_t fileSize, const FileType& fileType, string checksum,
		vector<uint64_t> objectList, vector<uint32_t> primaryList) {
	DownloadFileReplyMsg* downloadFileReplyMsg = new DownloadFileReplyMsg(this,
			requestId, connectionId, fileId, filePath, fileSize, fileType,
			checksum, objectList, primaryList);

	debug("FILESIZE = %" PRIu64 "\n", fileSize);

	downloadFileReplyMsg->prepareProtocolMsg();

	addMessage(downloadFileReplyMsg);
	return;
}

void MdsCommunicator::replyPrimary(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, uint32_t osdId) {

	SwitchPrimaryOsdReplyMsg* reply = new SwitchPrimaryOsdReplyMsg(this,
			requestId, connectionId, osdId);
	reply->prepareProtocolMsg();
	addMessage(reply);
	return;
}

/**
 * @brief	Reply Object ID List
 */
void MdsCommunicator::replyObjectIdList(uint32_t requestId,
		uint32_t connectionId, vector<uint64_t> objectList,
		vector<uint32_t> primaryList) {
	GetObjectIdListReplyMsg* getObjectIdListReplyMsg =
			new GetObjectIdListReplyMsg(this, requestId, connectionId,
					objectList, primaryList);
	getObjectIdListReplyMsg->prepareProtocolMsg();
	addMessage(getObjectIdListReplyMsg);
	return;
}

void MdsCommunicator::replyRecoveryTrigger(uint32_t requestId,
		uint32_t connectionId, vector<ObjectLocation> objectLocationList) {
	RecoveryTriggerReplyMsg * recoveryTriggerReplyMsg =
			new RecoveryTriggerReplyMsg(this, requestId, connectionId, objectLocationList);
	recoveryTriggerReplyMsg->prepareProtocolMsg();
	addMessage(recoveryTriggerReplyMsg);
}

void MdsCommunicator::reportFailure(uint32_t osdId, FailureReason reason) {
	return;
}

vector<bool> MdsCommunicator::getOsdStatusRequest(vector<uint32_t> osdIdList) {

	GetOsdStatusRequestMsg* getOsdStatusRequestMsg = new GetOsdStatusRequestMsg(
			this, getMonitorSockfd(), osdIdList);
	getOsdStatusRequestMsg->prepareProtocolMsg();

	addMessage(getOsdStatusRequestMsg, true);
	MessageStatus status = getOsdStatusRequestMsg->waitForStatusChange();

	if (status == READY) {
		vector<bool> osdStatusList = getOsdStatusRequestMsg->getOsdStatus();
		return osdStatusList;
	}

	return {};
}

