#include "../protocol/message.pb.h"
#include "communicator.hh"
#include <iostream>

using namespace std;

Communicator::Communicator() {
	// Verify version
//	GOOGLE_PROTOBUF_VERIFY_VERSION;
	cout << "Communicator Initialised" << endl;
}

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::waitForMessage() {
}
