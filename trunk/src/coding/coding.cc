#include "coding.hh"

Coding::Coding() {
}

Coding::~Coding() {

}

vector <struct SegmentData> Coding::performCoding(struct ObjectData objectData) {
	return _codingBehaviour->encode(objectData);
}


struct ObjectData Coding::performDecoding(vector <struct SegmentData> segmentData) {
	return _decodingBehaviour->decode(segmentData);
}

void Coding::setCodingBehaviour(CodingBehaviour *cb) {
	_codingBehaviour = cb;
}

void Coding::setDecodingBehaviour(DecodingBehaviour *db) {
	_decodingBehaviour = db;
}
