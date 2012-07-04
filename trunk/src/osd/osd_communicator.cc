#include <iostream>
#include "osd_communicator.hh"

using namespace std;

OsdCommunicator::~OsdCommunicator(){
	cout << "OSD Communicator Destroyed" << endl;

}

void OsdCommunicator::display(){
	cout << "OSD Communicator" << endl;
}

