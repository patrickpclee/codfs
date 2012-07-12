/**
 * memorypool.hh
 */

#ifndef __MEMORYPOOL_HH__
#define __MEMORYPOOL_HH__

/**
 * Provide a memory pool for optimizing frequent malloc / free calls
 */

class MemoryPool {
public:

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

	char* poolMalloc (uint32_t length);

	/**
	 * Retuen a piece of memory
	 * @param ptr Pointer to assigned memory
	 */

	void poolFree (char* ptr);
};

#endif
