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
	/**
	 * Constructor
	 */
	Monitor();

	/**
	 * Desctructor
	 */
	~Monitor();

	//getter functions

	/**
	 * Get a reference of MonitorCommunicator
	 * @return Pointer to Monitor communication module
	 */
	MonitorCommunicator* getCommunicator();

	/**
	 * Get a reference of Monitor's status module
	 * @return Pointer to statmodule
	 */
	StatModule* getStatModule();

	/**
	 * Get a reference of Monitor's recovery module
	 * @return Pointer to recoverymodule
	 */
	RecoveryModule* getRecoveryModule();

	/**
	 * Get the unique Monitor ID
	 * @return monitor id in uint32_t
	 */
	uint32_t getMonitorId();

	// Processors
	
	/**
	 * Action when an OSD startup message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param osdId OSD ID
	 * @param capacity Free space on the OSD
	 * @param loading Current CPU loading on the OSD
	 * @param ip OSD's ip address for other components to connect
	 * @param port OSD's port for other components to connect
	 */
	void OsdStartupProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading, uint32_t ip,
		uint16_t port);

	void getOsdConfigProcessor(uint32_t requestId, uint32_t sockfd);

	/**
	 * Action when an OSD update its status message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param osdId OSD ID
	 * @param capacity Free space on the OSD
	 * @param loading Current CPU loading on the OSD
	 */
	void OsdStatUpdateReplyProcessor(uint32_t requestId, uint32_t sockfd,
		uint32_t osdId, uint32_t capacity, uint32_t loading);
	
	/**
	 * Action when an OSD shutdown message received
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param osdId OSD ID
	 */
	void OsdShutdownProcessor(uint32_t requestId, uint32_t sockfd, uint32_t osdId);
	
	/**
	 * Action when an MDS request Primary OSDs for upload file
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param numOfObjs The number of OSDs required 
	 */
	void getPrimaryListProcessor(uint32_t requestId, uint32_t sockfd, uint32_t numOfObjs);
	
	/**
	 * Action when an OSD request Secondary OSDs for coding
	 * @param requestId Request ID
	 * @param sockfd Socket descriptor of message source
	 * @param numOfSegs Number of OSDs required for coding
	 */
	void getSecondaryListProcessor (uint32_t requestId, uint32_t sockfd, uint32_t numOfSegs);
	
private:
	
	/**
	 * Handles communication with other components
	 */
	MonitorCommunicator* _monitorCommunicator;

	/**
	 * Handles selection when request need a number of OSDs to process jobs
	 */
	SelectionModule* _selectionModule;
	
	/**
	 * Handles recovery fairs of OSDs
	 */
	RecoveryModule* _recoveryModule;
	
	/**
	 * Manage all the OSD status update fairs on this module
	 */
	StatModule* _statModule;

	/**
	 * the map used to store all the osd status
	 */	
	map<uint32_t, struct OsdStat> _osdStatMap;

	/**
	 * Unique ID for this monitor
	 */
	uint32_t _monitorId;
};

#endif
