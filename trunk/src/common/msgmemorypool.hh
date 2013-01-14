#ifndef __MSGMEMORYPOOL_HH__
#define __MSGMEMORYPOOL_HH__

#include "memorypool.hh"
#ifdef USE_APR_MEMORY_POOL
const apr_size_t MSG_POOL_MAX_FREE_SIZE = 1 * 1024 * 1024;
#endif

/**
 * Provide a memory pool for optimizing frequent malloc / free calls
 * TODO: A dummy memory pool using singleton pattern for now
 * TODO: Should be thread-safe in c++11 (verification needed)
 * Singleton Reference: http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
 */

class MsgMemoryPool: public MemoryPool {
public:
	static MsgMemoryPool& getInstance() {
		static MsgMemoryPool instance; // Guaranteed to be destroyed
									// Instantiated on first use
		return instance;
	}
	MsgMemoryPool();
	char* poolMalloc(uint32_t size);
private:
	uint32_t _maxMsgSize;
};

#endif
