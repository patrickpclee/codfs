#ifndef __RECVBUF_HH__
#define __RECVBUF_HH__

using namespace std;
#include "define.hh"

struct RecvBuffer {
	RecvBuffer() {
		len = 0;
		buf = (char*)malloc(RECV_BUF_PER_SOCKET);
	}
	uint32_t len;
	char* buf;
};

#endif
