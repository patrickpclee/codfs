#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <signal.h>
#include "monitor.hh"
#include "../config/config.hh"
#include "../common/blocklocation.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"

ConfigLayer* configLayer;

Monitor* monitor;

mutex osdStatMapMutex;
mutex osdLBMapMutex;

void sighandler(int signum) {
	cout << "Signal" << signum << "received" << endl;
	if (signum == SIGUSR1) {
		cout << "Try to recover failure" << endl;
		monitor->getRecoveryModule()->userTriggerDetection();
		cout << "done" << endl;
	}else if (signum == SIGUSR2) {
		cout << "Try to recover failure with destination specified" << endl;
		monitor->getRecoveryModule()->userTriggerDetection(true);
		cout << "done" << endl;
	}
}

/*  Monitor default constructor
 */
Monitor::Monitor() {
	srand(time(NULL));

	_osdStatMap = {};

	configLayer = new ConfigLayer("monitorconfig.xml");
	_monitorCommunicator = new MonitorCommunicator();
	_selectionModule = new SelectionModule(_osdStatMap, _osdLBMap);
	_statModule = new StatModule(_osdStatMap);
	_recoveryModule = new RecoveryModule(_osdStatMap, _monitorCommunicator);
	_monitorId = configLayer->getConfigInt("MonitorId");
	_sleepPeriod = configLayer->getConfigInt("SleepPeriod");
	_deadPeriod = configLayer->getConfigInt("DeadPeriod");
	_updatePeriod = configLayer->getConfigInt("UpdatePeriod");
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

RecoveryModule* Monitor::getRecoveryModule() {
	return _recoveryModule;
}

uint32_t Monitor::getMonitorId() {
	return _monitorId;
}

void Monitor::OsdStartupProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port) {

    lock_guard<mutex> lk(_osdStartUpProcessorMutex);

	debug(
			"OSD Startup Processor: on id = %" PRIu32 " ip = %" PRIu32 " port = %" PRIu32 "\n",
			osdId, ip, port);
	// Send online osd list to the newly startup osd
	vector<struct OnlineOsd> onlineOsdList;

	_statModule->getOnlineOsdList(onlineOsdList);
	_monitorCommunicator->sendOnlineOsdList(sockfd, onlineOsdList);

	// Send the newly startup osd stat to online osd
	_statModule->broadcastNewOsd(_monitorCommunicator, osdId, ip, port);

	// Add the newly startup osd to the map
	_statModule->setStatById(osdId, sockfd, capacity, loading, ONLINE, ip,
			port);

	// Add the newly startup osd to load balancing map
	_selectionModule->addNewOsdToLBMap(osdId);
}

void Monitor::OsdStatUpdateReplyProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading) {
	_statModule->setStatById(osdId, sockfd, capacity, loading, ONLINE);
}

void Monitor::OsdShutdownProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId) {
	_statModule->removeStatById(osdId);
}

void Monitor::getPrimaryListProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t numOfObjs) {
	vector<uint32_t> primaryList;
	primaryList = _selectionModule->choosePrimary(numOfObjs);
	_monitorCommunicator->replyPrimaryList(requestId, sockfd, primaryList);
	return;
}

void Monitor::getSecondaryListProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t numOfBlks, uint32_t primaryId, uint64_t blockSize) {

	vector<struct BlockLocation> secondaryList;
	secondaryList = _selectionModule->chooseSecondary(numOfBlks, primaryId, blockSize);
	_monitorCommunicator->replySecondaryList(requestId, sockfd, secondaryList);
	return;
}

void Monitor::getOsdListProcessor(uint32_t requestId, uint32_t sockfd) {
	vector<struct OnlineOsd> osdList;
	_statModule->getOnlineOsdList(osdList);
	_monitorCommunicator->replyOsdList(requestId, sockfd, osdList);
}

void Monitor::getOsdStatusRequestProcessor(uint32_t requestId, uint32_t sockfd,
		vector<uint32_t>& osdListRef) {
	vector<bool> osdStatus;
	_statModule->getOsdStatus(osdListRef, osdStatus);
	_monitorCommunicator->replyGetOsdStatus(requestId, sockfd, osdStatus);
}

uint32_t Monitor::getDeadPeriod() {
	return _deadPeriod;
}

uint32_t Monitor::getSleepPeriod() {
	return _sleepPeriod;
}

uint32_t Monitor::getUpdatePeriod() {
	return _updatePeriod;
}

int main(void) {
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	printf("MONITOR\n");

	monitor = new Monitor();
	MonitorCommunicator* communicator = monitor->getCommunicator();
	StatModule* statmodule = monitor->getStatModule();
#ifdef TRIGGER_RECOVERY
	RecoveryModule* recoverymodule = monitor->getRecoveryModule();
#endif

	// set up communicator
	communicator->setId(monitor->getMonitorId());
	communicator->setComponentType(MONITOR);
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);


	// 4. Update Thread
	thread updateThread(&StatModule::updateOsdStatMap, statmodule, communicator,
			monitor->getUpdatePeriod());

	// 5. Recovery Thread
#ifdef TRIGGER_RECOVERY
	thread recoveryThread(&RecoveryModule::failureDetection, recoverymodule,
			monitor->getDeadPeriod(), monitor->getSleepPeriod());
#endif

	// threads join
	garbageCollectionThread.join();
	receiveThread.join();
	updateThread.join();

#ifdef TRIGGER_RECOVERY
	recoveryThread.join();
#endif
	//clean up
	delete configLayer;
	delete monitor;

	return 0;
}
