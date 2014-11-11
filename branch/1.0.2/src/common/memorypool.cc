/**
 * memorypool.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "memorypool.hh"

MemoryPool::MemoryPool() {
}

MemoryPool::~MemoryPool() {
}

char* MemoryPool::poolMalloc(uint32_t length) {
	return (char*) calloc(length, 1);
}

void MemoryPool::poolFree(char* ptr) {
	free(ptr);
}
