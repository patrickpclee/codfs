#ifndef _OBJECTLOCATION_HH_ 
#define _OBJECTLOCATION_HH_ 

#include <vector>
using namespace std;

struct ObjectLocation {
	uint64_t objectId;
	uint32_t primaryId;
	vector<uint32_t> osdList;
};

#endif /* _OBJECTLOCATION_HH_*/
