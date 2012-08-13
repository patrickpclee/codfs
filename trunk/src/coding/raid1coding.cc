#include "coding.hh"
#include "raid1coding.hh"
#include "raid1encode.hh"
#include "raid1decode.hh"

Raid1Coding::Raid1Coding(uint32_t noOfReplications) {
	_noOfReplications = noOfReplications;
	_encodingBehaviour = new Raid1Encode(_noOfReplications);
	_decodingBehaviour = new Raid1Decode(_noOfReplications);
}

Raid1Coding::~Raid1Coding() {

}

void Raid1Coding::display() {

}
