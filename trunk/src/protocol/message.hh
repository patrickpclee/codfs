/**
 * message.hh
 */

#ifndef __MESSAGE_HH__
#define __MESSAGE_HH__

#include <string>
#include <stdint.h>
#include <future>

#include "../common/enums.hh"
#include "../communicator/communicator.hh"

class Communicator;

using namespace std;

/**
 * struct for message Header
 */

#pragma pack(1)
struct MsgHeader {
	uint32_t requestId;
	MsgType protocolMsgType;
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

	Message ();

	Message(Communicator* communicator);

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
	 * @param buf Buffer containing raw data
	 * @param length num of bytes to copy
	 * @return Num of bytes copied
	 */

	uint32_t preparePayload(char* buf, uint32_t length);


	//
	// for receive
	//

	/**
	 * Parse Message (binary) and store information into class variables
	 */

	virtual void parse(char* buf) = 0;

	/**
	 * After parsing, carry out action to handle the message
	 * handle() executes doHandle() and free memory afterwards
	 */

	void handle();

	/**
	 * Message-specific handler
	 */

	virtual void doHandle() = 0;

	//
	// setters
	//

	void setRequestId (uint32_t requestId);
	void setSockfd (uint32_t sockfd);
	void setProtocolType (MsgType protocolType);
	void setProtocolSize (uint32_t protocolSize);
	void setPayloadSize (uint32_t payloadSize);
	void setProtocolMsg(string protocolMsg);
	void setPayload(char* payload);
	void setRecvBuf (char* recvBuf);

	//
	// getter
	//

	uint32_t getSockfd();
	struct MsgHeader getMsgHeader ();
	string getProtocolMsg();
	char* getPayload();
	bool getExpectReply();


	/**
	 * @brief	Wait for Message Status Change
	 *
	 * @return	Status of the Message Reply
	 */
	MessageStatus waitForStatusChange();

	/**
	 * @brief	Set Message Status
	 *
	 * @param	status	New Status of the Message
	 */
	void setStatus (MessageStatus status);

	/**
	 * Set if message expects reply
	 * @param expectReply New expectReply value
	 */

	void setExpectReply (bool expectReply);

	/**
	 * DEBUG: Print the MsgHeader
	 */

	void printHeader();

	/**
	 * DEBUG: Print the protocol message in human-friendly way
	 */

	virtual void printProtocol() = 0;

	/**
	 * DEBUG: Dump the payload to stdout in hex format
	 */

	void printPayloadHex();

protected:
	uint32_t _sockfd;		// destination
	struct MsgHeader _msgHeader;
	string _protocolMsg;
	char* _payload;
	char* _recvBuf;		// buffer created to store header + protocol + payload in recvMessage
	bool _expectReply;
	Communicator* _communicator;
	
	promise <MessageStatus> _status;
};

#endif
