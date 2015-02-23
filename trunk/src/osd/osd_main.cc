/*
 * osd_main.cc
 */

#include <signal.h>
#include <thread>
#include <vector>
#include "osd.hh"
#include <iostream>
#include <iomanip>
using namespace std;
#include "../common/blocklocation.hh"
#include "../common/debug.hh"
#include "../common/garbagecollector.hh"
#include "../common/netfunc.hh"
#include "../protocol/status/getosdstatusrequestmsg.hh"
#include "../common/define.hh"
#include "../../lib/deathhandler/death_handler.h"

/// Osd Segment
Osd* osd;

/// Config Segment
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		exit(42);
	} else if (signum == SIGUSR1) {
		cout << "Dumping latency results...";
		fflush (stdout);
		osd->dumpLatency();
		cout << "done" << endl;
	}
}

/**
 * Main function
 * @return 0 if success;
 */

int main(int argc, char* argv[]) {

	signal(SIGINT, sighandler);
	signal(SIGUSR1, sighandler);

	// handle segFault for debug
	Debug::DeathHandler dh;
	(void) dh; // avoid warning

	char* interfaceName = NULL;
	char* gwIP = NULL;
//	bool forward = false;
	uint32_t selfId = 0;
	uint16_t selfPort = 0;

	if (argc < 3) {
		cout << "Usage: ./OSD [ID] [NETWORK INTERFACE] (Port)" << endl;
		exit(0);
	} else {
		selfId = atoi(argv[1]);
		interfaceName = argv[2];
		if (argc >= 4)
			selfPort = atoi(argv[3]);
	}

	// create new OSD segment and communicator
	osd = new Osd(selfId);

	// create new communicator
	OsdCommunicator* communicator = osd->getCommunicator();

	if (selfPort > 0)
		communicator->setServerPort((uint16_t)selfId);

	// set identity
	communicator->setId(osd->getOsdId());
	communicator->setComponentType(OSD);

	// create server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread([&](){GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	uint32_t selfAddr;
	selfAddr = getInterfaceAddressV4(interfaceName);
	selfPort = communicator->getServerPort();

	printIp(selfAddr);
	printf("Port = %hu\n",selfPort);

	//communicator->connectAllComponents();
	communicator->connectToMyself("127.0.0.1", selfPort, OSD);
	//communicator->connectToMyself(Ipv4Int2Str(selfAddr), selfPort, OSD);
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->registerToMonitor(selfAddr, selfPort);

	garbageCollectionThread.join();
	receiveThread.join();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}




