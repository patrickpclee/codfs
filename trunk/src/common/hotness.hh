#ifndef __HOTNESS_HH__
#define __HOTNESS_HH__

struct Hotness {
	Hotness() { }
	Hotness(double val, uint32_t ts): hotness(val), timestamp(ts) { } 
	double hotness;
	uint32_t timestamp;
};

#endif
