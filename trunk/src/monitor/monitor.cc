#include <cstdio>
#include "monitor.hh"
#include "../config/config.hh"

ConfigLayer* configLayer;

/*  Monitor default constructor
 */
Monitor::Monitor() {
	configLayer = new ConfigLayer("monitorconfig.xml");
	_selectionModule = new SelectionModule();
	_recoveryModule = new RecoveryModule();
	_statModule = new StatModule();
	_monitorCommunicator = new MonitorCommunicator();

	_osdStats.clear();
}

/*	Monitor default desctructor
 */
Monitor::~Monitor() {
	delete _selectionModule;
	delete _recoveryModule;
	delete _statModule;
	delete _monitorCommunicator;

	_osdStat.clear();
}

MonitorCommunicator* Monitor::getCommunicator() {
	return _monitorCommunicator;
}

int main (void) {
	
	printf ("MONITOR\n");

	Monitor* monitor = new Monitor();
	
	MonitorCommunicator* communicator = monitor->getCommunicator();

//	communicator->setId();
	communicator->setComponentType(MONITOR);

	communicator->createServerSocket();

	delete configLayer;
	delete monitor;

	return 0;
}
