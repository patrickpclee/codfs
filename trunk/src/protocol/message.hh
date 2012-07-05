#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include <stdint.h>

const uint32_t PAYLOADSIZE = 10*1024*1024; // 10MB

struct MsgHeader {
	uint32_t protocolMsgType;
	uint32_t protocolMsgSize;
	uint32_t payloadSize;
};

class Message {
public:
//	MessageHandler* messageHandler;

	Message();
	virtual ~Message();

	// for send
	void preparePayload (char* buf, uint32_t offset, uint32_t length);
	void prepareMsgHeader(uint32_t protocolMsgType, uint32_t protocolMsgSize);
	virtual void prepareProtocolMsg();

	// for receive
//	virtual void handler();
//	virtual void parseProtocolMsg();
//	void dumpPayload (char filepath[]);

private:
	struct MsgHeader _msgHeader;
	char* _protocolMsg;
	char* _payload;
};

#endif
