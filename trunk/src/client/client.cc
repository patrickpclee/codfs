#include <cstdio>
#include <thread>

#include "client.hh"
#include "../common/debug.hh"
#include "../config/config.hh"

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

Client::Client()
{
	_clientCommunicator = new ClientCommunicator();
}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* Client::getClientCommunicator()
{
	return _clientCommunicator;
}

void sendThread()
{
	debug("%s","Send Thread Start\n");
	client->getClientCommunicator()->sendMessage();
	debug("%s","Send Thread End\n");
}

int main (void) {

	configLayer = new ConfigLayer("clientconfig.xml");

	client = new Client();

	ClientCommunicator* communicator = client->getClientCommunicator();

	const uint16_t serverPort = configLayer->getConfigInt(
			"Communication>ServerPort");

	debug ("Start server on port %d\n", serverPort);

	communicator->createServerSocket(serverPort);

	// connect to MDS
	communicator->connectToMds();

	thread t (sendThread);
	t.detach();

	communicator->listFolderData(1,".");

	// wait for message
	communicator->waitForMessage();
	printf ("CLIENT\n");
	return 0;
}

