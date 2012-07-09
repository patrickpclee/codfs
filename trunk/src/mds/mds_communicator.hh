#ifndef __MDS_COMMUNICATOR_HH__
#define __MDS_COMMUNICATOR_HH__

#include "../communicator/communicator.hh"

#include <stdint.h>
#include <vector>
using namespace std;

class MdsCommunicator : public Communicator {
public:
	void display ();
	void sendObjectandPrimaryList(uint32_t clientId, vector<uint64_t> objectList, vector<uint32_t> primaryList);
private:
};
#endif
