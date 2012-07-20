#include <cstdio>
#include "client.hh"
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


int main (void) {

	configLayer = new ConfigLayer("clientconfig.xml");

	client = new Client();
	printf ("CLIENT\n");
	return 0;
}

