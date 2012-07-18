#ifndef __MDS_COMMUNICATOR_HH__
#define __MDS_COMMUNICATOR_HH__

#include "../communicator/communicator.hh"

#include <stdint.h>
#include <vector>
using namespace std;

class MdsCommunicator : public Communicator {
public:
	void display ();

	// Reply to Request
	void replyObjectandPrimaryList(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList, vector<uint32_t> primaryList);
	void replyNodeList(uint32_t requestId, uint32_t connectionId, uint32_t osdId, uint64_t objectId, vector<uint32_t>nodeList);
	void replyPrimary(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint64_t objectId, uint32_t osdId);
	void replyRecoveryInfo(uint32_t requestId, uint32_t connectionId, uint32_t monitorId, uint32_t osdId, vector<uint64_t> objectList, vector<uint32_t> primaryList, vector< vector<uint32_t> > objectNodeList);


	// Request to Other Nodes
	
	/**
	 * @brief	Report Failure to Monitor
	 * 
	 * @param	osdId	ID of the Failed OSD
	 * @param	reason	Reason of the Failure
	 */
	void reportFailure(uint32_t osdId, FailureReason reason);

	/**
	 * @brief	Ask Monitor for Primary List
	 *
	 * @param	numOfObjs	Number of Objects
	 */
	vector<uint32_t> askPrimaryList (uint32_t numOfObjs);

private:
};
#endif
