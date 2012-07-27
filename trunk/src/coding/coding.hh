#ifndef __CODING_HH__
#define __CODING_HH__

#include "codingbehaviour.hh"
#include "decodingbehaviour.hh"
#include "../osd/objectdata.hh"
#include "../osd/segmentdata.hh"

class Coding {
public:

	Coding();
	virtual ~Coding();

	vector <struct SegmentData> performCoding (struct ObjectData objectData);
	struct ObjectData performDecoding(vector <struct SegmentData> segmentData);
	void setCodingBehaviour(CodingBehaviour *cb);
	void setDecodingBehaviour(DecodingBehaviour *db);
	virtual void display() = 0; // make class abstract

	CodingBehaviour* _codingBehaviour;
	DecodingBehaviour* _decodingBehaviour;
};

#endif
