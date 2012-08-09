/**
 * memorypool.hh
 */

#ifndef __MEMORYPOOL_HH__
#define __MEMORYPOOL_HH__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <apr_lib.h>
#include <apr_buckets.h>
#include <apr_general.h>
#include <apr_pools.h>

/**
 * Provide a memory pool for optimizing frequent malloc / free calls
 * TODO: A dummy memory pool using singleton pattern for now
 * TODO: Should be thread-safe in c++11 (verification needed)
 * Singleton Reference: http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
 */


class MemoryPool {
public:

	/**
	 * static method for Singleton implementation
	 * @return reference to instance of singleton object
	 */

	static MemoryPool& getInstance() {
		static MemoryPool instance; // Guaranteed to be destroyed
									// Instantiated on first use
		return instance;
	}

	/**
	 * Constructor
	 */

	MemoryPool();

	/**
	 * Destructor
	 */

	~MemoryPool();

	/**
	 * Obtain a piece of memory
	 * @param length Length of memory
	 * @return Pointer to assigned memory
	 */

	char* poolMalloc(uint32_t length);

	/**
	 * Retuen a piece of memory
	 * @param ptr Pointer to assigned memory
	 */

	void poolFree(char* ptr);

private:
	// Dont forget to declare these two. You want to make sure they
	// are unaccessable otherwise you may accidently get copies of
	// your singleton appearing.

	MemoryPool(MemoryPool const&); // Don't Implement
	void operator=(MemoryPool const&); // Don't implement

	apr_pool_t *pool;
	apr_allocator_t* alloc;
	apr_bucket_alloc_t* balloc;
};

#endif
