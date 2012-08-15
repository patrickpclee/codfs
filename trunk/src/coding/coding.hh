#ifndef __CODING_HH__
#define __CODING_HH__

#include <vector>
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"

class Coding {
public:

	Coding();
	virtual ~Coding();

	virtual vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting) = 0;
	virtual struct ObjectData decode(vector<struct SegmentData> segmentData,
			string setting) = 0;

	virtual void display() = 0; // make class abstract
}
;

#endif
