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
	ListDirectoryRequestMsg* listDirectoryRequestMsg = new ListDirectoryRequestMsg(this,clientId,getMdsSockfd(), path);
	listDirectoryRequestMsg->prepareProtocolMsg();

	future< vector<FileMetaData> > folderData = listDirectoryRequestMsg->getFolderDataFuture();

	addMessage(listDirectoryRequestMsg,true);
	return folderData.get();
}

void ClientCommunicator::connectToMds()
{
	uint16_t port = 50000;
	string ip = "127.0.0.1";
	ComponentType connectionType = MDS;

	addConnection(ip, port, connectionType);

	return ;
}
