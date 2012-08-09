/**
 * memorypool.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>

#include "memorypool.hh"

std::mutex memoryPoolMutex;

MemoryPool::MemoryPool() {
	apr_initialize();
	apr_allocator_create(&alloc);
	apr_pool_create_ex(&pool, NULL, NULL, alloc);
	balloc = apr_bucket_alloc_create(pool);

}

MemoryPool::~MemoryPool() {
	apr_bucket_alloc_destroy(balloc);
	apr_pool_destroy(pool);
	apr_allocator_destroy(alloc);
	apr_terminate();
}

char* MemoryPool::poolMalloc(uint32_t length) {
	// TODO: use malloc for now, real pool implementation needed
//	return (char*)calloc(length, 1);

	std::lock_guard<std::mutex> lk(memoryPoolMutex);
	return (char *)apr_bucket_alloc((apr_size_t)length, balloc);
}

void MemoryPool::poolFree(char* ptr) {
	// TODO: use normal free for now, real pool implementation needed
//	free(ptr);

	std::lock_guard<std::mutex> lk(memoryPoolMutex);
	apr_bucket_free(ptr);
}
