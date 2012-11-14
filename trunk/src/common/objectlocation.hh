#ifndef _OBJECTLOCATION_HH_ 
#define _OBJECTLOCATION_HH_ 

#include <vector>
using namespace std;

struct ObjectLocation {
	uint64_t objectId;
	uint32_t primaryId;
	vector<uint32_t> osdList;
};

struct ObjectRepairInfo {
	uint64_t objectId;
	vector<uint32_t> repPos;
	vector<uint32_t> repOsd;
	void out() {
		printf("+--------ObjectRepairInfo----------\n");
		printf("| objectid | %24lld|\n", objectId);
		printf("| repPos = [");
		for (uint32_t i:repPos) printf("%d, ", i);
		printf("|\n");
		printf("| repOsd = [");
		for (uint32_t i:repOsd) printf("%d, ", i);
		printf("|\n");
		printf("+----------------------------------\n");
	}
};

#endif /* _OBJECTLOCATION_HH_*/
