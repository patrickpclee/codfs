#include <iostream>
#include "mds_communicator.hh"

#include "../common/debug.hh"
#include "../config/config.hh"
#include "../protocol/metadata/listdirectoryreply.hh"
#include "../protocol/metadata/uploadfilereply.hh"
#include "../protocol/nodelist/getprimarylistrequest.hh"
#include "../protocol/metadata/getobjectinforeply.hh"
#include "../protocol/metadata/downloadfilereply.hh"
#include "../protocol/metadata/getobjectidlistreply.hh"

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

void MdsCommunicator::display() {
	return;
}

void MdsCommunicator::replyObjectInfo(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, vector<uint32_t> nodeList, CodingScheme codingScheme,
		string codingSetting) {
	GetObjectInfoReplyMsg* getObjectInfoReplyMsg = new GetObjectInfoReplyMsg(
			this, requestId, connectionId, objectId, nodeList, codingScheme,
			codingSetting);
	getObjectInfoReplyMsg->prepareProtocolMsg();

	debug ("%s\n" , "======================");
	getObjectInfoReplyMsg->printHeader();
	getObjectInfoReplyMsg->printProtocol();
	debug ("%s\n" , "======================");

	addMessage(getObjectInfoReplyMsg);
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
		uint32_t connectionId, uint32_t fileId, string filePath, uint64_t fileSize,
		string checksum, vector<uint64_t> objectList,
		vector<uint32_t> primaryList) {
	DownloadFileReplyMsg* downloadFileReplyMsg = new DownloadFileReplyMsg(this,
			requestId, connectionId, fileId, filePath, fileSize, checksum, objectList,
			primaryList);

	debug ("FILESIZE = %" PRIu64 "\n", fileSize);

	downloadFileReplyMsg->prepareProtocolMsg();

	addMessage(downloadFileReplyMsg);
	return;
}

void MdsCommunicator::replyPrimary(uint32_t requestId, uint32_t connectionId,
		uint64_t objectId, uint32_t osdId) {
	return;
}

/**
 * @brief	Reply Object ID List
 */
void MdsCommunicator::replyObjectIdList(uint32_t requestId, uint32_t connectionId, vector<uint64_t> objectList, vector<uint32_t> primaryList)
{
	GetObjectIdListReplyMsg* getObjectIdListReplyMsg = new GetObjectIdListReplyMsg(this, requestId, connectionId, objectList, primaryList);
	getObjectIdListReplyMsg->prepareProtocolMsg();
	addMessage(getObjectIdListReplyMsg);
	return ;
}

void MdsCommunicator::replyRecoveryInfo(uint32_t requestId,
		uint32_t connectionId, uint32_t osdId, vector<uint64_t> objectList,
		vector<uint32_t> primaryList,
		vector<vector<uint32_t> > objectNodeList) {
	return;
}

void MdsCommunicator::reportFailure(uint32_t osdId, FailureReason reason) {
	return;
}

