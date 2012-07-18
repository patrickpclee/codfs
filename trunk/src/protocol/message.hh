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

#pragma pack(1)
struct MsgHeader {
	uint32_t requestId;
	uint32_t protocolMsgType;
	uint32_t protocolMsgSize;
	uint32_t payloadSize;
};
#pragma pack(0)

/**
 * Abstract class for all kinds of Message
 */

class Message {
public:

	/**
	 * Constructor
	 */

	Message();

	/**
	 * Destructor
	 */

	virtual ~Message();

	//
	// for send
	//

	/**
	 * Write the information contained in a Message class to a protocol string
	 */

	virtual void prepareProtocolMsg() = 0;

	/**
	 * Load the file contents into the payload
	 * @param filepath File to send
	 * @param offset Offset in file
	 * @param length num of bytes to copy
	 * @return Num of bytes copied
	 */

	uint32_t preparePayload(string filepath, uint32_t offset, uint32_t length);

	//
	// for receive
	//

	/**
	 * Parse Message (binary) and store information into class variables
	 */

	virtual void parse() = 0;

	/**
	 * After parsing, carry out action to handle the message
	 * (action should be run in separate thread)
	 */

	virtual void handle() = 0;

	//
	// setters
	//

	void setRequestId (uint32_t requestId);
	void setSockfd (uint32_t sockfd);
	void setProtocolType (MsgType protocolType);
	void setProtocolSize (uint32_t protocolSize);
	void setPayloadSize (uint32_t payloadSize);
	void setProtocolMsg(string protocolMsg);

	//
	// getter
	//

	uint32_t getSockfd();
	struct MsgHeader getMsgHeader ();
	string getProtocolMsg();
	char* getPayload();


	/**
	 * DEBUG: Print the MsgHeader
	 */

	void printHeader();

	/**
	 * DEBUG: Print the protocol message in human-friendly way
	 */

	virtual void printProtocol() = 0;

private:
	uint32_t _sockfd;		// destination
	struct MsgHeader _msgHeader;
	string _protocolMsg;
	char* _payload;
};

#endif
