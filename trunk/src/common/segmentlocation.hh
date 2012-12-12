#ifndef _SEGMENTLOCATION_HH_ 
#define _SEGMENTLOCATION_HH_ 

#include "../common/debug.hh"
#include <vector>
using namespace std;

struct SegmentLocation {
	uint64_t segmentId;
	uint32_t primaryId;
	vector<uint32_t> osdList;
};

struct SegmentRepairInfo {
	uint64_t segmentId;
	vector<uint32_t> repPos;
	vector<uint32_t> repOsd;
	void out() {
		printf("+--------SegmentRepairInfo----------\n");
		printf("| segmentid | %24" PRIu64 "|\n", segmentId);
		printf("| repPos = [");
		for (uint32_t i:repPos) printf("%" PRIu32 ", ", i);
		printf("|\n");
		printf("| repOsd = [");
		for (uint32_t i:repOsd) printf("%" PRIu32 ", ", i);
		printf("|\n");
		printf("+----------------------------------\n");
	}
};

#endif /* _SEGMENTLOCATION_HH_*/
