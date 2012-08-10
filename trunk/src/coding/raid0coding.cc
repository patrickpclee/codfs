#include "coding.hh"
#include "raid0coding.hh"
#include "raid0encode.hh"
#include "raid0decode.hh"

Raid0Coding::Raid0Coding() {
	_encodingBehaviour = new Raid0Encode();
	_decodingBehaviour = new Raid0Decode();
}

Raid0Coding::~Raid0Coding() {

}

void Raid0Coding::display() {

}
