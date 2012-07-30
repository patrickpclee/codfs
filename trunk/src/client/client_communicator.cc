#include "client_communicator.hh"

#include "../common/debug.hh"
#include "../protocol/listdirectoryrequest.hh"
#include "../protocol/putobjectinit.hh"

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
		return listDirectoryRequestMsg->getFolderData();
	} else {
		debug("%s\n","List Directory Request Failed");
		return {};
	}
	return {};
}

/**
 * 1. Send a putObjectInitMsg
 * 2. Send multiple putObjectDataMsg
 * 3. Send a putObjectDoneMsg
 */

void ClientCommunicator::uploadObject(uint32_t clientId, uint64_t objectId,
		string path, uint64_t offset, uint32_t length) {

	// Step 1

	PutObjectInitMsg * putObjectInitMsg = new PutObjectInitMsg(this,
			getOsdSockfd(), objectId, length);

	putObjectInitMsg->prepareProtocolMsg();
	addMessage(putObjectInitMsg, false);

	putObjectInitMsg->printHeader();
	putObjectInitMsg->printProtocol();



}

void ClientCommunicator::connectToMds() {
	uint16_t port = 50000;
	string ip = "127.0.0.1";
	ComponentType connectionType = MDS;

	connectAndAdd(ip, port, connectionType);

	return;
}

// TESTING
void ClientCommunicator::connectToOsd() {
	uint16_t port = 52000;
	string ip = "127.0.0.1";
	ComponentType connectionType = OSD;

	connectAndAdd(ip, port, connectionType);

	return;
}
