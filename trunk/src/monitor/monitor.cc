#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "monitor.hh"
#include "../config/config.hh"
#include "../common/segmentlocation.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"

ConfigLayer* configLayer;

Monitor* monitor;

mutex osdStatMapMutex;

/*  Monitor default constructor
 */
Monitor::Monitor() {
	srand(time(NULL));

	_osdStatMap = {};

	configLayer = new ConfigLayer("monitorconfig.xml");
	_monitorCommunicator = new MonitorCommunicator();
	_selectionModule = new SelectionModule(_osdStatMap);
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
	primaryList = _selectionModule->ChoosePrimary(numOfObjs);
	_monitorCommunicator->replyPrimaryList(requestId, sockfd, primaryList);
	return;
}

void Monitor::getSecondaryListProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t numOfSegs, uint32_t primaryId) {

	vector<struct SegmentLocation> secondaryList;
	secondaryList = _selectionModule->ChooseSecondary(numOfSegs, primaryId);
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

	printf("MONITOR\n");

	monitor = new Monitor();
	MonitorCommunicator* communicator = monitor->getCommunicator();
	StatModule* statmodule = monitor->getStatModule();
	RecoveryModule* recoverymodule = monitor->getRecoveryModule();

	// set up communicator
	communicator->setId(monitor->getMonitorId());
	communicator->setComponentType(MONITOR);
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	// 3. Send Thread
#ifdef USE_MULTIPLE_QUEUE
#else
	thread sendThread(&Communicator::sendMessage, communicator);
#endif

	// 4. Update Thread
	thread updateThread(&StatModule::updateOsdStatMap, statmodule, communicator,
			monitor->getUpdatePeriod());

	// 5. Recovery Thread
	thread recoveryThread(&RecoveryModule::failureDetection, recoverymodule,
			monitor->getDeadPeriod(), monitor->getSleepPeriod());

	// threads join
	garbageCollectionThread.join();
	receiveThread.join();
#ifdef USE_MULTIPLE_QUEUE
#else
	sendThread.join();
#endif
	updateThread.join();
	recoveryThread.join();
	//clean up
	delete configLayer;
	delete monitor;

	return 0;
}
