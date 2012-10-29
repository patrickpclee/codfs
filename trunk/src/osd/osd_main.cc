/*
 * osd_main.cc
 */

#include <signal.h>
#include <thread>
#include <vector>
#include "osd.hh"
#include "../common/segmentlocation.hh"
#include "../common/debug.hh"
#include "../common/garbagecollector.hh"
#include "../common/netfunc.hh"
#include "../protocol/status/getosdstatusrequestmsg.hh"
/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
		debug_yellow ("%s\n", "SIGINT received\n");
		exit(42);
	} else if (signum == SIGUSR1) {
		cout << "Clearing object disk cache...";
		fflush (stdout);
		osd->getStorageModule()->clearObjectDiskCache();
		cout << "done" << endl;
	}
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	osd->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	// wait for message
	communicator->waitForMessage();

}

void startTestThread(Communicator* communicator) {
	/*
	 printf("HEHE\n");
	 OsdStartupMsg* testmsg = new OsdStartupMsg(communicator,
	 communicator->getMonitorSockfd(), osd->getOsdId(), osd->getFreespace(),
	 osd->getCpuLoadavg(0));
	 testmsg->prepareProtocolMsg();
	 communicator->addMessage(testmsg);
	 sleep(1200);
	 OsdShutdownMsg* msg = new OsdShutdownMsg(communicator,
	 communicator->getMonitorSockfd(), osd->getOsdId());
	 msg->prepareProtocolMsg();
	 communicator->addMessage(msg);
	 printf("DONE\n");
	 */
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

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread);

	uint32_t selfAddr = getInterfaceAddressV4(interfaceName);
	uint16_t selfPort = communicator->getServerPort();
	printIp(selfAddr);
	printf("Port = %hu\n",selfPort);

	//communicator->connectAllComponents();
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->registerToMonitor(selfAddr, selfPort);

	//thread testThread(startTestThread, communicator);
	// TODO: pause before connect for now
	//getchar();

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();
	//testThread.join();

	// cleanup
	delete configLayer;
	delete osd;

	return 0;
}




