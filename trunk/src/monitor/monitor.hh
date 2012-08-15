#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "../cache/cache.hh"
#include "../common/osdstat.hh"
#include "monitor_communicator.hh"
#include "selectionmodule.hh"
#include "recoverymodule.hh"
#include "statmodule.hh"

#include <map>

using namespace std;

class Monitor {

public:
	// constructor
	Monitor();

	// desctructor
	~Monitor();

	// handle requests from MDS / OSD
//	uint32_t* primaryOsdListHandler(uint32_t objectCount);
//	uint32_t* secondaryOsdListHandler();
//	void osdFailureHandler(uint32_t osdId);

	// triggered by timeout
//	void osdRecoveryRequest(uint32_t osdId);
//	void recoveryResultHandler(uint32_t osdId, bool success);

	// Threads
	void startGarbageCollectionThread();
	void startSendThread(Communicator* communicator);
	void startReceiveThread(Communicator* communicator);
	void startUpdateThread(Communicator* communicator, StatModule* statmodule);

	// get methods
	MonitorCommunicator* getCommunicator();
	StatModule* getStatModule();
	uint32_t getMonitorId();

	void OsdStartupProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port);
	void OsdStatUpdateReplyProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading);
	void OsdShutdownProcessor(uint32_t requestId, uint32_t sockfd, uint32_t osdId);

	void getPrimaryListProcessor(uint32_t requestId, uint32_t connectionId, uint32_t numOfObjs);

private:
//	MonitorInfo _info;
//	Cache _cache;
	MonitorCommunicator* _monitorCommunicator;
	SelectionModule* _selectionModule;
	RecoveryModule* _recoveryModule;
	StatModule* _statModule;
	
	map<uint32_t, struct OsdStat> _osdStatMap;
	uint32_t _monitorId;
};

#endif
