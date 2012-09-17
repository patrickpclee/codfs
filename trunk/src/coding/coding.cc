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

	uint32_t* srcA32 = (uint32_t*) srcA;
	uint32_t* srcB32 = (uint32_t*) srcB;
	uint32_t* result32 = (uint32_t*) result;

	uint32_t xor32Count = length / sizeof(uint32_t);
	uint32_t offset = 0;

	// finish all the word-by-word XOR
	for (uint32_t i = 0; i < xor32Count; i++) {
		result32[i] = srcA32[i] ^ srcB32[i];
		offset = (i + 1) * sizeof(uint32_t); // processed bytes
	}

	// finish remaining byte-by-byte XOR
	for (uint32_t j = offset; j < length; j++) {
		result[j] = srcA[j] ^ srcB[j];
	}
}
