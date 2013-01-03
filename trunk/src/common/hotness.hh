#ifndef __HOTNESS_HH__
#define __HOTNESS_HH__

#include <ctime>
#include <vector>
#include "enums.hh"
using namespace std;

struct Hotness {
	Hotness() { 
		hotness = 0;
		type = COLD;
		timestamp = time(NULL);
	}
	double hotness;
	enum HotnessType type;
	uint32_t timestamp;
};

struct HotnessRequest {
	uint32_t numOfNewCache;
	vector<uint32_t> cachedOsdList;
};

#endif
