#include "coding.hh"

Coding::Coding() {
}

Coding::~Coding() {

}

vector <struct SegmentData> Coding::performEncoding(struct ObjectData objectData) {
	return _encodingBehaviour->encode(objectData);
}


struct ObjectData Coding::performDecoding(vector <struct SegmentData> segmentData) {
	return _decodingBehaviour->decode(segmentData);
}

void Coding::setEncodingBehaviour(EncodingBehaviour *cb) {
	_encodingBehaviour = cb;
}

void Coding::setDecodingBehaviour(DecodingBehaviour *db) {
	_decodingBehaviour = db;
}
