#include <cstdio>
#include "client.hh"

Client* client;

Client::Client()
{
	_clientCommunicator = new ClientCommunicator();
}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* getClientCommunicator()
{
	return _clientCommunicator;
}


int main (void) {

	client = new Client();
	printf ("CLIENT\n");
	return 0;
}

