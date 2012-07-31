#include <iostream>
#include "client_communicator.hh"

#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../protocol/listdirectoryrequest.hh"

#include "../protocol/putobjectinitrequest.hh"
#include "../protocol/objectdatamsg.hh"
#include "../protocol/putobjectend.hh"

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

void ClientCommunicator::putObject(uint32_t clientId, uint32_t dstOsdSockfd,
		struct ObjectData objectData) {

	const uint64_t totalSize = objectData.info.objectSize;
	const uint64_t objectId = objectData.info.objectId;
	char* buf = objectData.buf;


	// Step 1 : Send Init message

	putObjectInit(clientId, dstOsdSockfd, objectId, totalSize);
	debug ("%s\n", "Put Object Init Sent");

	// Step 2 : Send data chunk by chunk

	uint64_t byteToSend = 0;
	uint64_t byteProcessed = 0;
	uint64_t byteRemaining = totalSize;

	while (byteProcessed < totalSize) {

		if (byteRemaining > _chunkSize) {
			byteToSend = _chunkSize;
		} else {
			byteToSend = byteRemaining;
		}

		putObjectData(clientId, dstOsdSockfd, objectId, buf, byteProcessed,
				byteToSend);
		byteProcessed += byteToSend;
		byteRemaining -= byteToSend;

//		usleep (100*1000);
	}

	sleep (1);

	debug ("%s\n", "All Put Object Data Sent");

	// Step 3: Send End message

	putObjectEnd(clientId, dstOsdSockfd, objectId);

	debug ("%s\n", "Put Object End Sent");

	cout << "Put Object Finished" << endl;

}

//
// PRIVATE FUNCTIONS
//

void ClientCommunicator::putObjectInit(uint32_t clientId, uint32_t dstOsdSockfd,
		uint64_t objectId, uint32_t length) {

	// Step 1 of the upload process

	PutObjectInitRequestMsg* putObjectInitRequestMsg = new PutObjectInitRequestMsg(this,
			dstOsdSockfd, objectId, length);

	putObjectInitRequestMsg->prepareProtocolMsg();
	addMessage(putObjectInitRequestMsg, true);

	MessageStatus status = putObjectInitRequestMsg->waitForStatusChange();
	if(status == READY) {
		return;
	} else {
		debug("%s\n", "Put Object Init Failed");
		exit (-1);
	}

}

void ClientCommunicator::putObjectData(uint32_t clientID, uint32_t dstOsdSockfd,
		uint64_t objectId, char* buf, uint64_t offset, uint32_t length) {

	// Step 2 of the upload process
	ObjectDataMsg* objectDataMsg = new ObjectDataMsg(this, dstOsdSockfd, objectId, offset, length);

	objectDataMsg->prepareProtocolMsg();
	objectDataMsg->preparePayload(buf + offset, length);

	addMessage(objectDataMsg, false);

}

void ClientCommunicator::putObjectEnd(uint32_t clientId, uint32_t dstOsdSockfd,
		uint64_t objectId) {

	// Step 3 of the upload process

	PutObjectEndMsg* putObjectEndMsg = new PutObjectEndMsg (this, dstOsdSockfd, objectId);

	putObjectEndMsg->prepareProtocolMsg();
	addMessage(putObjectEndMsg, false);
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
