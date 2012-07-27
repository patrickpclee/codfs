#include "coding.hh"
#include "dummycoding.hh"
#include "dummyencode.hh"
#include "dummydecode.hh"

DummyCoding::DummyCoding() {
	_encodingBehaviour = new DummyEncode();
	_decodingBehaviour = new DummyDecode();
}

DummyCoding::~DummyCoding() {

}

void DummyCoding::display() {

}
