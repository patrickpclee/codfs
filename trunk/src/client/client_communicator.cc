#include "client_communicator.hh"

#include "../protocol/listdirectoryrequest.hh"

vector<FileMetaData> ClientCommunicator::listFolderData (uint32_t clientId, string path)
{
	ListDirectoryRequestMsg* listDirectoryRequestMsg = new ListDirectoryRequestMsg(clientId,getMdsSockfd(), path);
	listDirectoryRequestMsg->prepareProtocolMsg();

	future< vector<FileMetaData> > folderData = listDirectoryRequestMsg->getFolderDataFuture();

	addMessage(listDirectoryRequestMsg);
	return folderData.get();
}
