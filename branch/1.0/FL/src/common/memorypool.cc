/**
 * memorypool.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "memorypool.hh"

MemoryPool::MemoryPool() {
#ifdef USE_APR_MEMORY_POOL
	apr_initialize();
	apr_allocator_create(&alloc);
	apr_allocator_max_free_set(alloc, POOL_MAX_FREE_SIZE);
	apr_pool_create_ex(&pool, NULL, NULL, alloc);
	balloc = apr_bucket_alloc_create(pool);
#endif
}

MemoryPool::~MemoryPool() {
#ifdef USE_APR_MEMORY_POOL
	apr_bucket_alloc_destroy(balloc);
	apr_pool_destroy(pool);
	apr_allocator_destroy(alloc);
	apr_terminate();
#endif
}

char* MemoryPool::poolMalloc(uint32_t length) {
#ifdef USE_APR_MEMORY_POOL
	{
		std::lock_guard<std::mutex> lk(memoryPoolMutex);
		char* buf = (char *) apr_bucket_alloc((apr_size_t) length, balloc);
		memset(buf, 0, length);
		return buf;
	}
#elif defined USE_NEDMALLOC
	return (char*) nedalloc::nedcalloc (length, 1);
#else
	return (char*) calloc(length, 1);
#endif
}

void MemoryPool::poolFree(char* ptr) {
#ifdef USE_APR_MEMORY_POOL
	{
		std::lock_guard<std::mutex> lk(memoryPoolMutex);
		apr_bucket_free(ptr);
	}
#elif defined USE_NEDMALLOC
	return nedalloc::nedfree(ptr);
#else
	free(ptr);
#endif

}
