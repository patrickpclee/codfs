#include <iostream>
#include <algorithm>
#include "mds_communicator.hh"
#include "../common/debug.hh"
#include "../config/config.hh"
#include "../protocol/metadata/listdirectoryreply.hh"
#include "../protocol/metadata/uploadfilereply.hh"
#include "../protocol/metadata/savesegmentlistreply.hh"
#include "../protocol/metadata/deletefilereply.hh"
#include "../protocol/nodelist/getprimarylistrequest.hh"
#include "../protocol/metadata/getsegmentinforeply.hh"
#include "../protocol/metadata/downloadfilereply.hh"
#include "../protocol/metadata/getsegmentidlistreply.hh"
#include "../protocol/metadata/renamefilereply.hh"
#include "../protocol/status/getosdstatusrequestmsg.hh"
#include "../protocol/status/recoverytriggerreply.hh"
#include "../protocol/metadata/uploadsegmentackreply.hh"
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
void MdsCommunicator::replyRenameFile(uint32_t requestId, uint32_t connectionId,
		uint32_t fileId, const string& path) {
	RenameFileReplyMsg* renameFileReplyMsg = new RenameFileReplyMsg(this,
			requestId, connectionId, fileId);
	renameFileReplyMsg->prepareProtocolMsg();
	addMessage(renameFileReplyMsg);
	return;
}

void MdsCommunicator::replySegmentInfo(uint32_t requestId,
		uint32_t connectionId, uint64_t segmentId, uint32_t segmentSize,
		vector<uint32_t> nodeList, CodingScheme codingScheme,
		string codingSetting) {
	GetSegmentInfoReplyMsg* getSegmentInfoReplyMsg = new GetSegmentInfoReplyMsg(
			this, requestId, connectionId, segmentId, segmentSize, nodeList,
			codingScheme, codingSetting);
	getSegmentInfoReplyMsg->prepareProtocolMsg();

	debug("%s\n", "======================");
	getSegmentInfoReplyMsg->printHeader();
	getSegmentInfoReplyMsg->printProtocol();
	debug("%s\n", "======================");

	addMessage(getSegmentInfoReplyMsg);
	return;
}

/**
 * @brief	Reply Save Segment List Request
 */
void MdsCommunicator::replySaveSegmentList(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId) {
	SaveSegmentListReplyMsg* saveSegmentListReplyMsg =
			new SaveSegmentListReplyMsg(this, requestId, connectionId, fileId);
	saveSegmentListReplyMsg->prepareProtocolMsg();
	addMessage(saveSegmentListReplyMsg);
	return;
}

/**
 * @brief	Reply Segment and Primary List to Client
 */
void MdsCommunicator::replySegmentandPrimaryList(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId, vector<uint64_t> segmentList,
		vector<uint32_t> primaryList) {
	UploadFileReplyMsg* uploadFileReplyMsg = new UploadFileReplyMsg(this,
			requestId, connectionId, fileId, segmentList, primaryList);
	uploadFileReplyMsg->prepareProtocolMsg();

	addMessage(uploadFileReplyMsg);
	return;
}

/**
 * @brief	Reply Download Information to Client
 *
 * File Size, Segment List, Primary List
 */
void MdsCommunicator::replyDownloadInfo(uint32_t requestId,
		uint32_t connectionId, uint32_t fileId, string filePath,
		uint64_t fileSize, const FileType& fileType,
		vector<uint64_t> segmentList, vector<uint32_t> primaryList) {
	DownloadFileReplyMsg* downloadFileReplyMsg = new DownloadFileReplyMsg(this,
			requestId, connectionId, fileId, filePath, fileSize, fileType,
			segmentList, primaryList);

	debug("FILESIZE = %" PRIu64 "\n", fileSize);

	downloadFileReplyMsg->prepareProtocolMsg();

	addMessage(downloadFileReplyMsg);
	return;
}

/**
 * @brief	Reply Segment ID List
 */
void MdsCommunicator::replySegmentIdList(uint32_t requestId,
		uint32_t connectionId, vector<uint64_t> segmentList,
		vector<uint32_t> primaryList) {
	GetSegmentIdListReplyMsg* getSegmentIdListReplyMsg =
			new GetSegmentIdListReplyMsg(this, requestId, connectionId,
					segmentList, primaryList);
	getSegmentIdListReplyMsg->prepareProtocolMsg();
	addMessage(getSegmentIdListReplyMsg);
	return;
}

void MdsCommunicator::replyRecoveryTrigger(uint32_t requestId,
		uint32_t connectionId, vector<SegmentLocation> segmentLocationList) {
	RecoveryTriggerReplyMsg * recoveryTriggerReplyMsg =
			new RecoveryTriggerReplyMsg(this, requestId, connectionId,
					segmentLocationList);
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


void MdsCommunicator::replyUploadSegmentAck(uint32_t requestId, 
    uint32_t connectionId, uint64_t segmentId)  {

    UploadSegmentAckReplyMsg* uploadSegmentAckReplyMsg = 
        new UploadSegmentAckReplyMsg(this, requestId, connectionId, segmentId);
    uploadSegmentAckReplyMsg->prepareProtocolMsg();
    uploadSegmentAckReplyMsg->printProtocol();
    addMessage(uploadSegmentAckReplyMsg, false);
}
