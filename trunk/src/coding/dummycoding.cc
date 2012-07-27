#include "coding.hh"
#include "dummycoding.hh"
#include "dummyencode.hh"
#include "dummydecode.hh"

DummyCoding::DummyCoding() {
	_codingBehaviour = new DummyEncode();
	_decodingBehaviour = new DummyDecode();
}
