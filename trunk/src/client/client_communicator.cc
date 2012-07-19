#include "client_communicator.hh"

#include "../protocol/listdirectoryrequest.hh"

/**
 * @brief	Send List Folder Request to MDS (Blocking)
 *
 * 1. Create List Directory Request Message \n
 * 2. Use Future to Wait until Reply is Back
 */
vector<FileMetaData> ClientCommunicator::listFolderData (uint32_t clientId, string path)
{
	ListDirectoryRequestMsg* listDirectoryRequestMsg = new ListDirectoryRequestMsg(clientId,getMdsSockfd(), path);
	listDirectoryRequestMsg->prepareProtocolMsg();

	future< vector<FileMetaData> > folderData = listDirectoryRequestMsg->getFolderDataFuture();

	addMessage(listDirectoryRequestMsg);
	return folderData.get();
}
