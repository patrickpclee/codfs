#ifndef __MONITOR_COMMUNICATOR_HH__
#define __MONITOR_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include <vector>
#include "../communicator/communicator.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all MONITOR communications
 */

class MonitorCommunicator: public Communicator {
public:

	/**
	 * Constructor
	 */
	MonitorCommunicator();

	/**
	 * Destructor
	 */
	~MonitorCommunicator();

	/**
	 * Action to reply a request from MDS for primary OSD list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param primaryList List of selected primary osd IDs 
	 */
	void replyPrimaryList(uint32_t requestId, uint32_t sockfd, 
		vector<uint32_t> primaryList);

	/**
	 * Action to reply a request from OSD for secondary OSD list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param primaryList List of selected secondary osd IDs 
	 */
	void replySecondaryList(uint32_t requestId, uint32_t sockfd, 
		vector<struct SegmentLocation> secondaryList);

private:

};

#endif
