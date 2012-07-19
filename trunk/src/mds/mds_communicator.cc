#include "mds_communicator.hh"

#include "../protocol/listdirectoryreply.hh"


/**
 * @brief	Reply With Folder Data
 */
void MdsCommunicator::replyFolderData(uint32_t requestId, uint32_t connectionId, string path, vector<FileMetaData> folderData)
{
	ListDirectoryReplyMsg listDirectoryReplyMsg* = new ListDirectoryReplyMsg(requestId, connectionId, path, folderData);
	listDirectoryReplyMsg->prepareProtocolMsg();
	
	addMessage(listDirectoryReplyMsg);
	return ;
}
