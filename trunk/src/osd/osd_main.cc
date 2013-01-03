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

#ifdef TIME_POINT
extern double lockSegmentCountMutexTime;
extern double getSegmentInfoTime;
extern double getOSDStatusTime;
extern double getBlockTime;
extern double decodeSegmentTime;
extern double sendSegmentTime;
extern double cacheSegmentTime;
#endif

/// Osd Segment
Osd* osd;

/// Config Segment
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		cout << fixed;
		cout << setprecision(2);
#ifdef TIME_POINT
cout << "Lock Segment Count Mutex: " << lockSegmentCountMutexTime / 1000 << endl;
cout << "Get Segment Info: " << getSegmentInfoTime / 1000 << endl;
cout << "Get OSD Status: " << getOSDStatusTime / 1000 << endl;
cout << "Get Block: " << getBlockTime / 1000 << endl;
cout << "Decode Segment: " << decodeSegmentTime / 1000 << endl;
cout << "Send Segment: " << sendSegmentTime / 1000 << endl;
cout << "Cache Segment: " << cacheSegmentTime / 1000 << endl;
#endif
		exit(42);
	} else if (signum == SIGUSR1) {
		cout << "Clearing segment disk cache...";
		fflush (stdout);
		osd->getStorageModule()->clearSegmentDiskCache();
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

	char* interfaceName = NULL;
	uint32_t selfId = 0;

	if (argc < 3) {
		cout << "Usage: ./OSD [ID] [NETWORK INTERFACE]" << endl;
		exit(0);
	} else {
		selfId = atoi(argv[1]);
		interfaceName = argv[2];
	}

	// create new OSD segment and communicator
	osd = new Osd(selfId);

	// create new communicator
	OsdCommunicator* communicator = osd->getCommunicator();

	// set identity
	communicator->setId(osd->getOsdId());
	communicator->setComponentType(OSD);

	// create server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread([&](){GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	// 3. Send Thread
#ifdef USE_MULTIPLE_QUEUE
#else
	thread sendThread(&Communicator::sendMessage, communicator);
#endif

	// 4. Cache Thread
	thread cacheThread (&Osd::reportRemovedCache, osd);

	uint32_t selfAddr = getInterfaceAddressV4(interfaceName);
	uint16_t selfPort = communicator->getServerPort();
	printIp(selfAddr);
	printf("Port = %hu\n",selfPort);

	//communicator->connectAllComponents();
	communicator->connectToMyself(Ipv4Int2Str(selfAddr), selfPort, OSD);
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->registerToMonitor(selfAddr, selfPort);

	garbageCollectionThread.join();
	receiveThread.join();
#ifdef USE_MULTIPLE_QUEUE
#else
	sendThread.join();
#endif
	//testThread.join();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}




