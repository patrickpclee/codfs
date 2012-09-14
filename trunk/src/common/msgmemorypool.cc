#include "msgmemorypool.hh"

#include "enums.hh"

#include "../protocol/message.hh"

MsgMemoryPool::MsgMemoryPool() {
	_maxMsgSize = 0;
	for (uint32_t i = 1; i < MSGTYPE_END; ++i) { // Skip DEFAULT
		Message* tempMessage = MessageFactory::createMessage(NULL, (MsgType) i);
		delete tempMessage;
	}
}

char* MsgMemoryPool::poolMalloc(uint32_t size) {
	if (size > _maxMsgSize)
		_maxMsgSize = size;

#ifdef USE_MEMORY_POOL
	return MemoryPool::poolMalloc(_maxMsgSize);
#else
	return (char*)malloc (size);
#endif
}

