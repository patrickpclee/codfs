/**
 * memorypool.cc
 */

#include <stdlib.h>
#include "memorypool.hh"

MemoryPool::MemoryPool() {

}

MemoryPool::~MemoryPool() {

}

char* MemoryPool::poolMalloc(uint32_t length) {
	// TODO: use malloc for now, real pool implementation needed
	return (char*)malloc(length);
}

void MemoryPool::poolFree(char* ptr) {
	// TODO: use normal free for now, real pool implementation needed
	free(ptr);
}
