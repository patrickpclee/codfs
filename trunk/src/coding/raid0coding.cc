#include "coding.hh"
#include "raid0coding.hh"
#include "raid0encode.hh"
#include "raid0decode.hh"

Raid0Coding::Raid0Coding(uint32_t noOfStrips) {
	_noOfStrips = noOfStrips;
	_encodingBehaviour = new Raid0Encode(_noOfStrips);
	_decodingBehaviour = new Raid0Decode(_noOfStrips);
}

Raid0Coding::~Raid0Coding() {

}

void Raid0Coding::display() {

}
