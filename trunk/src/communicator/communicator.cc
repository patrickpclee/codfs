#include <iostream>
#include "connection.hh"
#include "../common/enums.hh"
#include "communicator.hh"
#include "../protocol/message.pb.h"

using namespace std;

Communicator::Communicator() {
	cout << "Checking Google Protocol Buffer Version...";
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	cout << "Success" << endl;
	cout << "Communicator Initialised" << endl;
}

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::waitForMessage() {
}

void Communicator::addConnection(string ip, uint16_t port, ComponentType connectionType) {
	Connection conn (ip, port, connectionType);
	_connectionList.push_back(conn);
}
