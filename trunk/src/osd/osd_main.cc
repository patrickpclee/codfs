/*
 * osd_main.cc
 */

#include <signal.h>
#include <thread>
#include <vector>
#include "osd.hh"
#include "segmentlocation.hh"
#include "../common/debug.hh"
#include "../common/garbagecollector.hh"

/// Osd Object
Osd* osd;

/// Config Object
ConfigLayer* configLayer;

// handle ctrl-C for profiler
void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGINT) {
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

	string configFilePath;

	if (argc < 2) {
		cout << "Usage: ./OSD [OSD CONFIG FILE]" << endl;
		exit(0);
	} else {
		configFilePath = string(argv[1]);
	}

	// create new OSD object and communicator
	osd = new Osd(configFilePath);

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

	communicator->connectAllComponents();
	communicator->connectToMonitor();
	communicator->registerToMonitor();

	debug("%s\n", "starting test thread");
	sleep(5);

	if (osd->getOsdId() == 52000) {
//		osd->getObjectRequestProcessor(0, 0, 83937998);
	}

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




