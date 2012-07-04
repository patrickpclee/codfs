#include "communicator.hh"
#include <iostream>

using namespace std;

Communicator::~Communicator() {
	cout << "Communicator Destroyed" << endl;
}

void Communicator::waitForMessage() {
}
