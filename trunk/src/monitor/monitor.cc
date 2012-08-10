#include <cstdio>
#include "monitor.hh"
#include "../config/config.hh"
#include "../common/garbagecollector.hh"

ConfigLayer* configLayer;


mutex osdStatMapMutex;

/*  Monitor default constructor
 */
Monitor::Monitor() {
	_osdStatMap = {};

	configLayer = new ConfigLayer("monitorconfig.xml");
	_selectionModule = new SelectionModule(_osdStatMap);
	_recoveryModule = new RecoveryModule();
	_statModule = new StatModule(_osdStatMap);
	_monitorCommunicator = new MonitorCommunicator();
	_monitorId = configLayer->getConfigInt("MonitorId");
}

/*	Monitor default desctructor
 */
Monitor::~Monitor() {
	delete _selectionModule;
	delete _recoveryModule;
	delete _statModule;
	delete _monitorCommunicator;
}

MonitorCommunicator* Monitor::getCommunicator() {
	return _monitorCommunicator;
}

StatModule* Monitor::getStatModule() {
	return _statModule;
}

uint32_t Monitor::getMonitorId() {
	return _monitorId;
}


void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread(Communicator* communicator) {
	communicator->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	communicator->waitForMessage();
}

void startUpdateThread(Communicator* communicator, StatModule* statmodule) {
	statmodule->updateOsdStatMap(communicator);
}

int main (void) {
	
	printf ("MONITOR\n");

	Monitor* monitor = new Monitor();
	MonitorCommunicator* communicator = monitor->getCommunicator();
	StatModule* statmodule = monitor->getStatModule();

	// set up communicator
	communicator->setId(monitor->getMonitorId());
	communicator->setComponentType(MONITOR);

	communicator->createServerSocket();

	// 1. Garbage Collection Thread
	thread garbageCollectionThread(startGarbageCollectionThread);

	// 2. Receive Thread
	thread receiveThread(startReceiveThread, communicator);

	// 3. Send Thread
	thread sendThread(startSendThread, communicator);

	// 4. Update Thread
	thread updateThread(startUpdateThread, communicator, statmodule);

	// threads join
	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();
	updateThread.join();

	//clean up
	delete configLayer;
	delete monitor;

	return 0;
}
