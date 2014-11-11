#ifndef __RECVBUF_HH__
#define __RECVBUF_HH__

using namespace std;
#include "define.hh"
#include "../common/memorypool.hh"

struct RecvBuffer {
	RecvBuffer() {
		len = 0;
		buf = MemoryPool::getInstance().poolMalloc(RECV_BUF_PER_SOCKET);
	}
	~RecvBuffer() {
		MemoryPool::getInstance().poolFree(buf);
	}
	uint32_t len;
	char* buf;
};

#endif
