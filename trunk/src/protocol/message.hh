/**
 * message.hh
 */

#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include <string>
#include <stdint.h>
#include "../common/enums.hh"

using namespace std;

/**
 * struct for message Header
 */
struct MsgHeader {
	uint32_t protocolMsgType;
	uint32_t protocolMsgSize;
	uint32_t payloadSize;
};

/**
 * Abstract class for all kinds of Message
 */
class Message {
public:

	Message();
	virtual ~Message();

	// for send
	virtual void prepareProtocolMsg() = 0;
	void preparePayload(string filepath, uint32_t offset, uint32_t length);

	// for debug
	void printHeader();
	virtual void printProtocol() = 0;

	// for receive
//	MessageHandler* messageHandler;
//	static Message* createMessageFromHeader (MsgHeader _msgHeader);
//	virtual void handler();
//	virtual void parseProtocolMsg() = 0;

	// setters
	void setSockfd (uint32_t sockfd);
	void setProtocolType (MsgType protocolType);
	void setProtocolSize (uint32_t protocolSize);
	void setPayloadSize (uint32_t payloadSize);
	void setProtocolMsg(string protocolMsg);

private:
	struct MsgHeader _msgHeader;
	string _protocolMsg;
	char* _payload;
	uint32_t _sockfd;	// use when sending message to destination
};

#endif
