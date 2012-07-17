#ifndef __MDS_COMMUNICATOR_HH__
#define __MDS_COMMUNICATOR_HH__

#include "../communicator/communicator.hh"

#include <stdint.h>
#include <vector>
using namespace std;

class MdsCommunicator : public Communicator {
public:
	void display ();

	void sendObjectandPrimaryList(uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList, vector<uint32_t> primaryList);
	void sendNodeList(uint32_t osdId, uint64_t objectId, vector<uint32_t>nodeList);
	void reportFailure(uint32_t osdId, FailureReason reason);
	void sendPrimary(uint32_t clientId, uint64_t objectId, uint32_t osdId);
	void sendRecoveryInfo(uint32_t monitorId, uint32_t osdId, vector<uint64_t> objectList, vector<uint32_t> primaryList, vector< vector<uint32_t> > objectNodeList);
private:
};
#endif
