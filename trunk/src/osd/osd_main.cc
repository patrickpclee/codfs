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
#include "../common/segmentlocation.hh"
#include "../common/debug.hh"
#include "../common/garbagecollector.hh"
#include "../common/netfunc.hh"
#include "../protocol/status/getosdstatusrequestmsg.hh"
#include "../common/define.hh"

#ifdef TIME_POINT
extern double lockObjectCountMutexTime;
extern double getObjectInfoTime;
extern double getOSDStatusTime;
extern double getSegmentTime;
extern double decodeObjectTime;
extern double sendObjectTime;
extern double cacheObjectTime;
#endif

/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		cout << fixed;
		cout << setprecision(2);
#ifdef TIME_POINT
cout << "Lock Object Count Mutex: " << lockObjectCountMutexTime / 1000 << endl;
cout << "Get Object Info: " << getObjectInfoTime / 1000 << endl;
cout << "Get OSD Status: " << getOSDStatusTime / 1000 << endl;
cout << "Get Segment: " << getSegmentTime / 1000 << endl;
cout << "Decode Object: " << decodeObjectTime / 1000 << endl;
cout << "Send Object: " << sendObjectTime / 1000 << endl;
cout << "Cache Object: " << cacheObjectTime / 1000 << endl;
#endif
		exit(42);
	} else if (signum == SIGUSR1) {
		cout << "Clearing object disk cache...";
		fflush (stdout);
		osd->getStorageModule()->clearObjectDiskCache();
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

	// create new OSD object and communicator
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

	uint32_t selfAddr = getInterfaceAddressV4(interfaceName);
	uint16_t selfPort = communicator->getServerPort();
	printIp(selfAddr);
	printf("Port = %hu\n",selfPort);

	//communicator->connectAllComponents();
	communicator->connectToMyself(Ipv4Int2Str(selfAddr), selfPort, OSD);
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->registerToMonitor(selfAddr, selfPort);

	//thread testThread(startTestThread, communicator);
	// TODO: pause before connect for now
	//getchar();

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




