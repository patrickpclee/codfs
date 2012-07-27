#ifndef __CODING_HH__
#define __CODING_HH__

#include "encodingbehaviour.hh"
#include "decodingbehaviour.hh"
#include "../osd/objectdata.hh"
#include "../osd/segmentdata.hh"

class Coding {
public:

	Coding();
	virtual ~Coding();

	vector <struct SegmentData> performEncoding (struct ObjectData objectData);
	struct ObjectData performDecoding(vector <struct SegmentData> segmentData);
	void setEncodingBehaviour(EncodingBehaviour *cb);
	void setDecodingBehaviour(DecodingBehaviour *db);
	virtual void display() = 0; // make class abstract

	EncodingBehaviour* _encodingBehaviour;
	DecodingBehaviour* _decodingBehaviour;
};

#endif
