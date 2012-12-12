#ifndef __MONITOR_COMMUNICATOR_HH__
#define __MONITOR_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include <vector>
#include "../communicator/communicator.hh"
#include "../common/onlineosd.hh"

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
	 * @param secondaryList List of selected secondary osd IDs 
	 */
	void replySecondaryList(uint32_t requestId, uint32_t sockfd, 
		vector<struct BlockLocation> secondaryList);

	/**
	 * Action to reply a request from CLIENT for online OSD list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param osdList List of online osd 
	 */
	void replyOsdList(uint32_t requestId, uint32_t sockfd, 
		vector<struct OnlineOsd>& osdList);

	/**
	 * Action to reply a request from CLIENT for online OSD list
	 * @param requestId Request ID
	 * @param sockfd Socket ID between the connection
	 * @param osdStatus boolean list of osd status
	 */
	void replyGetOsdStatus(uint32_t requestId, uint32_t sockfd,
		vector<bool>& osdStatusRef);

	/**
	 * Action to send current online OSDs to the newly start one
	 * @param newOsdSockfd Socket ID of the newly start OSD
	 * @param onlineOsdList List of online OSDs with their ip,port,id 
	 */
	void sendOnlineOsdList(uint32_t newOsdSockfd, 
		vector<struct OnlineOsd>& onlineOsdList);

private:

};

#endif
