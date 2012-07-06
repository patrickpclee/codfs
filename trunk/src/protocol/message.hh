#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include <stdint.h>

const uint32_t PAYLOADSIZE = 10*1024*1024; // 10MB

struct MsgHeader {
	uint32_t protocolMsgType;
	uint32_t protocolMsgSize;
	uint32_t payloadSize;
};

// Abstract Base Class
class Message {
public:

	Message();
	virtual ~Message();

	// for send
	virtual void prepareProtocolMsg() = 0;
	void setProtocolType (MsgType protocolType);
	void setProtocolSize (uint32_t protocolSize);
	void setPayloadSize (uint32_t payloadSize);
	void prepareMsgHeader(uint32_t protocolMsgType, uint32_t protocolMsgSize);
	void preparePayload (char* buf, uint32_t offset, uint32_t length);

	// for debug
	void printHeader();
	virtual void printProtocol() = 0;

	// for receive
//	virtual void handler();
//	virtual void parseProtocolMsg();
//	void dumpPayload (char filepath[]);

//	MessageHandler* messageHandler;

private:
	struct MsgHeader _msgHeader;
	char* _protocolMsg;
	char* _payload;
};

#endif
