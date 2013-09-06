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

uint32_t Coding::getParityCountFromSetting(string setting) {
    return 0;
}

// default function, can be overridden
vector<BlockData> Coding::computeDelta(BlockData oldBlock, BlockData newBlock,
        vector<offset_length_t> offsetLength, vector<uint32_t> parityBlockIdVector) {

    uint32_t combinedLength = getCombinedLength(offsetLength);
    vector<BlockData> deltas (parityBlockIdVector.size());

    for (int i = 0; i < (int)parityBlockIdVector.size(); i++) {
        BlockData &delta = deltas[i];
        delta = oldBlock;
        delta.info.blockId = parityBlockIdVector[i];
        delta.buf = MemoryPool::getInstance().poolMalloc(combinedLength);
        bitwiseXor(delta.buf, oldBlock.buf, newBlock.buf, combinedLength);
    }
    return deltas;
}

uint32_t Coding::getCombinedLength(vector<offset_length_t> offsetLength) {
    uint32_t combinedLength = 0;
    for (auto offsetLengthPair : offsetLength) {
        combinedLength += offsetLengthPair.second;
    }
    return combinedLength;
}
