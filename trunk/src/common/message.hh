#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include <stdint.h>

struct MsgHeader {
	uint32_t protocolMsgType;
	uint32_t protocolMsgSize;
	uint32_t payloadSize;
};

class Message {
public:
	MessageHandler* messageHandler;

	// for send
	virtual void prepareProtocolMsg();
	void preparePayload (char buf[], uint32_t offset, uint32_t length);
	void prepareMsgHeader();

	// for receive
	virtual void handler();
	virtual void parseProtocolMsg();
	void dumpPayload (char filepath[]);

private:
	struct MsgHeader msgHeader;
	char protocolMsg[];
	char payload[];
};

#endif
