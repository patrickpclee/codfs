#include "coding.hh"
#include "../common/debug.hh"
#include "../common/memorypool.hh"

Coding::Coding() {
}

Coding::~Coding() {

}


uint32_t Coding::roundTo(uint32_t numToRound, uint32_t multiple) {
	if (multiple == 0) {
		return numToRound;
	}

	uint32_t remainder = numToRound % multiple;
	if (remainder == 0)
		return numToRound;
	return numToRound + multiple - remainder;
}

void Coding::bitwiseXor(char* result, char* srcA, char* srcB, uint32_t length) {

	uint64_t* srcA64 = (uint64_t*) srcA;
	uint64_t* srcB64 = (uint64_t*) srcB;
	uint64_t* result64 = (uint64_t*) result;

	uint64_t xor64Count = length / sizeof(uint64_t);
	uint64_t offset = 0;

	// finish all the word-by-word XOR
	for (uint64_t i = 0; i < xor64Count; i++) {
		result64[i] = srcA64[i] ^ srcB64[i];
		offset += sizeof(uint64_t); // processed bytes
	}

	// finish remaining byte-by-byte XOR
	for (uint64_t j = offset; j < length; j++) {
		result[j] = srcA[j] ^ srcB[j];
	}
}
