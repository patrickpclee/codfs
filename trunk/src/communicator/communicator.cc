#include <iostream>
#include "communicator.hh"
#include "../protocol/message.pb.h"

using namespace std;

Communicator::Communicator() {
	cout << "Communicator Initialised" << endl;
}

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::waitForMessage() {
}
