#include <cstdio>
#include <thread>

#include "client.hh"
#include "../common/debug.hh"
#include "../config/config.hh"

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

void* processor;

Client::Client() {
	_clientCommunicator = new ClientCommunicator();
	processor = (void*) this;
}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* Client::getCommunicator() {
	return _clientCommunicator;
}

void sendThread() {
	debug("%s", "Send Thread Start\n");
	client->getCommunicator()->sendMessage();
	debug("%s", "Send Thread End\n");
}

void sleepThread(ClientCommunicator* communicator) {

	usleep(2000000);
	vector<FileMetaData> folderData;
	folderData = communicator->listFolderData(1, ".");

	// TODO: when to delete listFolderDataRequest and listFolderDataReply?

	vector<FileMetaData>::iterator it;
	for (it = folderData.begin(); it < folderData.end(); ++it) {
		debug("name: %s size: %d\n", ((*it)._path).c_str(), (int)(*it)._size);
	}
}

int main(void) {

	configLayer = new ConfigLayer("clientconfig.xml");

	client = new Client();

	ClientCommunicator* communicator = client->getCommunicator();

	const uint16_t serverPort = configLayer->getConfigInt(
			"Communication>ServerPort");

	debug("Start server on port %d\n", serverPort);

	communicator->createServerSocket(serverPort);

	// connect to MDS
	communicator->connectToMds();

	thread t(sendThread);
	t.detach();
	thread t1(sleepThread, communicator);
	t1.detach();

	// wait for message
	communicator->waitForMessage();

	printf("CLIENT\n");
	return 0;
}

