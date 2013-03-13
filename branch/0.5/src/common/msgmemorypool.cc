#include "msgmemorypool.hh"

#include "enums.hh"

#include "../common/debug.hh"
#include "../protocol/message.hh"

MsgMemoryPool::MsgMemoryPool() {
	_maxMsgSize = 0;
}

char* MsgMemoryPool::poolMalloc(uint32_t size) {
	if (size > _maxMsgSize)
		_maxMsgSize = size;

	return MemoryPool::poolMalloc(_maxMsgSize);
}

