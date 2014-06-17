/**
 * memorypool.hh
 */

#ifndef __MEMORYPOOL_HH__
#define __MEMORYPOOL_HH__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../common/define.hh"

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
	 * @return reference to instance of singleton segment
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

	char* poolMallocMsg();

private:
	// Dont forget to declare these two. You want to make sure they
	// are unaccessable otherwise you may accidently get copies of
	// your singleton appearing.

	MemoryPool(MemoryPool const&); // Don't Implement
	void operator=(MemoryPool const&); // Don't implement

	uint32_t _maxMsgSize;
};
#endif
